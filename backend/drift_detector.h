#ifndef DRIFT_DETECTOR_H
#define DRIFT_DETECTOR_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_PATH_LEN 4096
#define MAX_FILES 100
#define SOCKET_PATH "/tmp/drift_detector.sock"
#define CONFIG_FILE_PATH ".config/drift_detector/config.json"
#define BASELINE_DIR ".config/drift_detector/baselines"

typedef struct {
    char path[MAX_PATH_LEN];
    bool monitoring;
    int wd; // inotify watch descriptor
} MonitoredFile;

typedef struct {
    MonitoredFile files[MAX_FILES];
    int count;
    char smtp_server[256];
    int smtp_port;
    char smtp_user[256];
    char smtp_pass[256];
    char alert_email[256];
} Config;

// Configuration functions
int load_config(Config *config);
int save_config(const Config *config);

// File monitoring functions
int init_inotify(void);
int add_watch(int fd, const char *path);
void monitor_files(int inotify_fd, Config *config);

// Drift detection functions
int create_baseline(const char *filepath);
int detect_drift(const char *filepath, char *diff_output, size_t max_len);

// Alert functions
int send_email_alert(const Config *config, const char *filepath, const char *diff);

// Socket server functions
int start_socket_server(Config *config);
void handle_client_command(int client_fd, Config *config, int inotify_fd);

// Daemon functions
void daemonize(void);

#endif // DRIFT_DETECTOR_H
