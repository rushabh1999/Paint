#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

int main(int argc, char *argv[]) {
    Config config;
    int inotify_fd, socket_fd;
    int daemon_mode = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0) {
            daemon_mode = 1;
        }
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Load configuration
    if (load_config(&config) < 0) {
        fprintf(stderr, "Error loading configuration\n");
        return 1;
    }
    
    // Initialize inotify
    inotify_fd = init_inotify();
    if (inotify_fd < 0) {
        fprintf(stderr, "Failed to initialize inotify\n");
        return 1;
    }
    
    // Add watches for all configured files
    for (int i = 0; i < config.count; i++) {
        config.files[i].wd = add_watch(inotify_fd, config.files[i].path);
    }
    
    // Start socket server
    socket_fd = start_socket_server(&config);
    if (socket_fd < 0) {
        fprintf(stderr, "Failed to start socket server\n");
        close(inotify_fd);
        return 1;
    }
    
    if (daemon_mode) {
        printf("Starting daemon mode...\n");
        daemonize();
    }
    
    printf("Drift Detector running. Monitoring %d file(s)\n", config.count);
    
    // Main loop
    while (running) {
        fd_set readfds;
        struct timeval tv;
        int max_fd;
        
        FD_ZERO(&readfds);
        FD_SET(inotify_fd, &readfds);
        FD_SET(socket_fd, &readfds);
        
        max_fd = (inotify_fd > socket_fd) ? inotify_fd : socket_fd;
        
        // Set timeout to 1 second
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (ret < 0) {
            if (!running) break;
            perror("select");
            continue;
        }
        
        if (ret == 0) {
            // Timeout, continue
            continue;
        }
        
        // Check inotify events
        if (FD_ISSET(inotify_fd, &readfds)) {
            monitor_files(inotify_fd, &config);
        }
        
        // Check socket connections
        if (FD_ISSET(socket_fd, &readfds)) {
            int client_fd = accept(socket_fd, NULL, NULL);
            if (client_fd >= 0) {
                handle_client_command(client_fd, &config, inotify_fd);
                close(client_fd);
            }
        }
    }
    
    printf("\nShutting down...\n");
    
    // Cleanup
    close(inotify_fd);
    close(socket_fd);
    unlink(SOCKET_PATH);
    
    return 0;
}
