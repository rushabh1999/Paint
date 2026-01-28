#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void daemonize(void) {
    pid_t pid;
    
    // Fork first child
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Exit parent
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    // Create new session
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Fork second child
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Exit first child
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    // Set file permissions
    umask(0);
    
    // Change working directory
    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Redirect to /dev/null or log file
    int fd = open("/dev/null", O_RDWR);
    if (fd >= 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) {
            close(fd);
        }
    }
}
