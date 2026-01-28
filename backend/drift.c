#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>

static void get_baseline_path(const char *filepath, char *baseline_path, size_t size) {
    const char *home = getenv("HOME");
    char *path_copy, *filename;
    
    if (!home) home = ".";
    
    path_copy = strdup(filepath);
    filename = basename(path_copy);
    
    snprintf(baseline_path, size, "%s/%s/%s", home, BASELINE_DIR, filename);
    
    free(path_copy);
}

int create_baseline(const char *filepath) {
    char baseline_path[MAX_PATH_LEN];
    char command[MAX_PATH_LEN * 2];
    
    get_baseline_path(filepath, baseline_path, sizeof(baseline_path));
    
    // Ensure baseline directory exists
    char dir_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(dir_path, sizeof(dir_path), "%s/%s", home, BASELINE_DIR);
    
    // Copy file to baseline
    snprintf(command, sizeof(command), "cp '%s' '%s'", filepath, baseline_path);
    
    if (system(command) != 0) {
        fprintf(stderr, "Error creating baseline for %s\n", filepath);
        return -1;
    }
    
    printf("Baseline created for %s\n", filepath);
    return 0;
}

int detect_drift(const char *filepath, char *diff_output, size_t max_len) {
    char baseline_path[MAX_PATH_LEN];
    char command[MAX_PATH_LEN * 2 + 100];
    FILE *fp;
    size_t bytes_read;
    
    get_baseline_path(filepath, baseline_path, sizeof(baseline_path));
    
    // Check if baseline exists
    if (access(baseline_path, F_OK) != 0) {
        return -1; // No baseline found
    }
    
    // Use diff to compare files
    snprintf(command, sizeof(command), "diff -u '%s' '%s' 2>&1", baseline_path, filepath);
    
    fp = popen(command, "r");
    if (!fp) {
        perror("popen");
        return -1;
    }
    
    bytes_read = fread(diff_output, 1, max_len - 1, fp);
    diff_output[bytes_read] = '\0';
    
    int status = pclose(fp);
    
    // diff returns 0 if files are identical, 1 if different, 2 on error
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0) {
            return -1; // No drift
        } else if (exit_code == 1) {
            return 0; // Drift detected
        }
    }
    
    return -1; // Error or no drift
}
