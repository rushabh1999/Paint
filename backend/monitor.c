#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <errno.h>
#include <time.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

int init_inotify(void) {
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        perror("inotify_init1");
    }
    return fd;
}

int add_watch(int fd, const char *path) {
    int wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CLOSE_WRITE);
    if (wd < 0) {
        fprintf(stderr, "Error adding watch for %s: %s\n", path, strerror(errno));
    } else {
        printf("Added watch for %s (wd=%d)\n", path, wd);
    }
    return wd;
}

void monitor_files(int inotify_fd, Config *config) {
    char buffer[EVENT_BUF_LEN];
    char diff_output[8192];
    int i, length;
    struct inotify_event *event;
    time_t now;
    char time_str[64];
    
    length = read(inotify_fd, buffer, EVENT_BUF_LEN);
    
    if (length < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read");
        }
        return;
    }
    
    i = 0;
    while (i < length) {
        event = (struct inotify_event *)&buffer[i];
        
        if (event->mask & (IN_MODIFY | IN_CLOSE_WRITE)) {
            // Find which file was modified
            for (int j = 0; j < config->count; j++) {
                if (config->files[j].wd == event->wd) {
                    time(&now);
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
                    
                    printf("[%s] Change detected in: %s\n", time_str, config->files[j].path);
                    
                    // Detect drift
                    if (detect_drift(config->files[j].path, diff_output, sizeof(diff_output)) == 0) {
                        printf("Drift detected! Sending alert...\n");
                        
                        // Send email alert
                        if (send_email_alert(config, config->files[j].path, diff_output) == 0) {
                            printf("Alert sent successfully\n");
                        } else {
                            fprintf(stderr, "Failed to send alert\n");
                        }
                    } else {
                        printf("No drift detected or baseline not found\n");
                    }
                    
                    break;
                }
            }
        }
        
        i += EVENT_SIZE + event->len;
    }
}
