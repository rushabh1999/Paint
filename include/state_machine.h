#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>
#include <time.h>

typedef enum {
    STATE_UNKNOWN,
    STATE_ONLINE,
    STATE_OFFLINE
} ConnectionState;

typedef struct {
    ConnectionState current_state;
    ConnectionState previous_state;
    time_t outage_start;
    time_t outage_end;
    bool outage_detected;
} StateMachine;

void init_state_machine(StateMachine* sm);
void update_state(StateMachine* sm, bool is_connected);
bool is_state_changed(StateMachine* sm);
long get_outage_duration(StateMachine* sm);

#endif
