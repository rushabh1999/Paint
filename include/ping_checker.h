#ifndef PING_CHECKER_H
#define PING_CHECKER_H

#include <stdbool.h>

typedef struct {
    bool is_connected;
    long latency_ms;
} PingResult;

PingResult check_connectivity(const char* host);
void init_ping_checker(void);
void cleanup_ping_checker(void);

#endif
