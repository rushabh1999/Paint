#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/inotify.h>
#include <errno.h>
#include <fcntl.h>

int start_socket_server(Config *config) {
    (void)config; // Unused parameter
    int server_fd;
    struct sockaddr_un addr;
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }
    
    // Make socket non-blocking
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    // Remove existing socket file
    unlink(SOCKET_PATH);
    
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }
    
    printf("Socket server listening on %s\n", SOCKET_PATH);
    
    return server_fd;
}

void handle_client_command(int client_fd, Config *config, int inotify_fd) {
    char buffer[4096];
    char response[4096];
    int n;
    
    n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        return;
    }
    
    buffer[n] = '\0';
    
    printf("Received command: %s\n", buffer);
    
    // Parse command
    char *cmd = strtok(buffer, " \n");
    
    if (!cmd) {
        snprintf(response, sizeof(response), "ERROR: Empty command\n");
        if (write(client_fd, response, strlen(response)) < 0) {
            perror("write");
        }
        return;
    }
    
    if (strcmp(cmd, "ADD") == 0) {
        char *filepath = strtok(NULL, "\n");
        if (!filepath) {
            snprintf(response, sizeof(response), "ERROR: No file specified\n");
        } else {
            // Check if file already exists
            int exists = 0;
            for (int i = 0; i < config->count; i++) {
                if (strcmp(config->files[i].path, filepath) == 0) {
                    exists = 1;
                    break;
                }
            }
            
            if (exists) {
                snprintf(response, sizeof(response), "ERROR: File already monitored\n");
            } else if (config->count >= MAX_FILES) {
                snprintf(response, sizeof(response), "ERROR: Maximum files limit reached\n");
            } else {
                strncpy(config->files[config->count].path, filepath, MAX_PATH_LEN - 1);
                config->files[config->count].monitoring = true;
                config->files[config->count].wd = add_watch(inotify_fd, filepath);
                config->count++;
                save_config(config);
                snprintf(response, sizeof(response), "OK: File added to monitoring\n");
            }
        }
    } else if (strcmp(cmd, "REMOVE") == 0) {
        char *filepath = strtok(NULL, "\n");
        if (!filepath) {
            snprintf(response, sizeof(response), "ERROR: No file specified\n");
        } else {
            int found = -1;
            for (int i = 0; i < config->count; i++) {
                if (strcmp(config->files[i].path, filepath) == 0) {
                    found = i;
                    break;
                }
            }
            
            if (found == -1) {
                snprintf(response, sizeof(response), "ERROR: File not found in monitoring list\n");
            } else {
                // Remove inotify watch
                if (config->files[found].wd >= 0) {
                    inotify_rm_watch(inotify_fd, config->files[found].wd);
                }
                
                // Shift array
                for (int i = found; i < config->count - 1; i++) {
                    config->files[i] = config->files[i + 1];
                }
                config->count--;
                save_config(config);
                snprintf(response, sizeof(response), "OK: File removed from monitoring\n");
            }
        }
    } else if (strcmp(cmd, "BASELINE") == 0) {
        char *filepath = strtok(NULL, "\n");
        if (!filepath) {
            snprintf(response, sizeof(response), "ERROR: No file specified\n");
        } else {
            if (create_baseline(filepath) == 0) {
                snprintf(response, sizeof(response), "OK: Baseline created\n");
            } else {
                snprintf(response, sizeof(response), "ERROR: Failed to create baseline\n");
            }
        }
    } else if (strcmp(cmd, "STATUS") == 0) {
        if (config->count == 0) {
            snprintf(response, sizeof(response), "No files being monitored\n");
        } else {
            int pos = 0;
            pos += snprintf(response + pos, sizeof(response) - pos, "Monitored files:\n");
            for (int i = 0; i < config->count && (size_t)pos < sizeof(response) - 100; i++) {
                pos += snprintf(response + pos, sizeof(response) - pos, 
                    "  [%d] %s (wd=%d)\n", i + 1, config->files[i].path, config->files[i].wd);
            }
        }
    } else if (strcmp(cmd, "CONFIGURE") == 0) {
        // Parse SMTP settings from command
        char *server = strtok(NULL, "|");
        char *port_str = strtok(NULL, "|");
        char *user = strtok(NULL, "|");
        char *pass = strtok(NULL, "|");
        char *email = strtok(NULL, "\n");
        
        if (server && port_str && user && pass && email) {
            strncpy(config->smtp_server, server, 255);
            config->smtp_port = atoi(port_str);
            strncpy(config->smtp_user, user, 255);
            strncpy(config->smtp_pass, pass, 255);
            strncpy(config->alert_email, email, 255);
            save_config(config);
            snprintf(response, sizeof(response), "OK: SMTP settings updated\n");
        } else {
            snprintf(response, sizeof(response), "ERROR: Invalid SMTP settings format\n");
        }
    } else {
        snprintf(response, sizeof(response), "ERROR: Unknown command\n");
    }
    
    if (write(client_fd, response, strlen(response)) < 0) {
        perror("write");
    }
}
