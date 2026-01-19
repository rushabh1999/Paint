#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static FILE* log_file = NULL;
static char current_log_path[MAX_PATH];
static char log_directory[MAX_PATH];

static void get_date_string(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d", tm_info);
}

static void create_log_filename(char* buffer, size_t size, const char* log_dir) {
    char date_str[32];
    get_date_string(date_str, sizeof(date_str));
    snprintf(buffer, size, "%s\\uptime_log_%s.csv", log_dir, date_str);
}

bool init_logger(const char* log_dir) {
    if (log_dir == NULL) return false;
    
    strncpy(log_directory, log_dir, MAX_PATH - 1);
    log_directory[MAX_PATH - 1] = '\0';
    
    // Create log directory if it doesn't exist
    CreateDirectoryA(log_dir, NULL);
    
    create_log_filename(current_log_path, sizeof(current_log_path), log_dir);
    
    // Check if file exists to determine if we need to write header
    bool file_exists = (GetFileAttributesA(current_log_path) != INVALID_FILE_ATTRIBUTES);
    
    log_file = fopen(current_log_path, "a");
    if (log_file == NULL) {
        return false;
    }
    
    // Write CSV header if new file
    if (!file_exists) {
        fprintf(log_file, "Timestamp,Status,Latency_ms,Outage_Duration_sec\n");
        fflush(log_file);
    }
    
    return true;
}

bool log_entry(const LogEntry* entry) {
    if (log_file == NULL || entry == NULL) return false;
    
    // Check if we need to rotate to a new log file for a new day
    char new_log_path[MAX_PATH];
    create_log_filename(new_log_path, sizeof(new_log_path), log_directory);
    
    if (strcmp(new_log_path, current_log_path) != 0) {
        // Close old file and open new one
        fclose(log_file);
        strncpy(current_log_path, new_log_path, MAX_PATH - 1);
        
        bool file_exists = (GetFileAttributesA(current_log_path) != INVALID_FILE_ATTRIBUTES);
        log_file = fopen(current_log_path, "a");
        if (log_file == NULL) return false;
        
        if (!file_exists) {
            fprintf(log_file, "Timestamp,Status,Latency_ms,Outage_Duration_sec\n");
        }
    }
    
    // Format timestamp
    char time_str[64];
    struct tm* tm_info = localtime(&entry->timestamp);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Write log entry
    fprintf(log_file, "%s,%s,%ld,%ld\n",
            time_str,
            entry->is_connected ? "UP" : "DOWN",
            entry->latency_ms,
            entry->outage_duration_sec);
    
    fflush(log_file);
    return true;
}

void close_logger(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

const char* get_current_log_file(void) {
    return current_log_path;
}
