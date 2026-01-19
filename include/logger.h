#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include <time.h>

typedef struct {
    time_t timestamp;
    bool is_connected;
    long latency_ms;
    long outage_duration_sec;
} LogEntry;

bool init_logger(const char* log_dir);
bool log_entry(const LogEntry* entry);
void close_logger(void);
const char* get_current_log_file(void);

#endif
