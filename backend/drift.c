#include "drift_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <ctype.h>

// Escape shell special characters in file paths
static void escape_path_for_shell(const char *input, char *output, size_t output_size) {
    size_t i, j = 0;
    
    // Add opening quote
    if (j < output_size - 1) {
        output[j++] = '\'';
    }
    
    for (i = 0; input[i] != '\0' && j < output_size - 3; i++) {
        if (input[i] == '\'') {
            // Escape single quote as '\''
            if (j < output_size - 5) {
                output[j++] = '\'';
                output[j++] = '\\';
                output[j++] = '\'';
                output[j++] = '\'';
            }
        } else {
            output[j++] = input[i];
        }
    }
    
    // Add closing quote
    if (j < output_size - 1) {
        output[j++] = '\'';
    }
    output[j] = '\0';
}

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
    char escaped_src[MAX_PATH_LEN * 2];
    char escaped_dst[MAX_PATH_LEN * 2];
    char command[MAX_PATH_LEN * 4];
    
    get_baseline_path(filepath, baseline_path, sizeof(baseline_path));
    
    // Ensure baseline directory exists
    char dir_path[MAX_PATH_LEN];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(dir_path, sizeof(dir_path), "%s/%s", home, BASELINE_DIR);
    
    // Escape paths for shell command
    escape_path_for_shell(filepath, escaped_src, sizeof(escaped_src));
    escape_path_for_shell(baseline_path, escaped_dst, sizeof(escaped_dst));
    
    // Copy file to baseline
    snprintf(command, sizeof(command), "cp %s %s", escaped_src, escaped_dst);
    
    if (system(command) != 0) {
        fprintf(stderr, "Error creating baseline for %s\n", filepath);
        return -1;
    }
    
    printf("Baseline created for %s\n", filepath);
    return 0;
}

int detect_drift(const char *filepath, char *diff_output, size_t max_len) {
    char baseline_path[MAX_PATH_LEN];
    char escaped_baseline[MAX_PATH_LEN * 2];
    char escaped_file[MAX_PATH_LEN * 2];
    char command[MAX_PATH_LEN * 4 + 100];
    FILE *fp;
    size_t bytes_read;
    
    get_baseline_path(filepath, baseline_path, sizeof(baseline_path));
    
    // Check if baseline exists
    if (access(baseline_path, F_OK) != 0) {
        return -1; // No baseline found
    }
    
    // Escape paths for shell command
    escape_path_for_shell(baseline_path, escaped_baseline, sizeof(escaped_baseline));
    escape_path_for_shell(filepath, escaped_file, sizeof(escaped_file));
    
    // Use diff to compare files
    snprintf(command, sizeof(command), "diff -u %s %s 2>&1", escaped_baseline, escaped_file);
    
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
