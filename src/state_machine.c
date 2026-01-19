#include "state_machine.h"
#include <string.h>

void init_state_machine(StateMachine* sm) {
    if (sm == NULL) return;
    
    sm->current_state = STATE_UNKNOWN;
    sm->previous_state = STATE_UNKNOWN;
    sm->outage_start = 0;
    sm->outage_end = 0;
    sm->outage_detected = false;
}

void update_state(StateMachine* sm, bool is_connected) {
    if (sm == NULL) return;
    
    sm->previous_state = sm->current_state;
    sm->current_state = is_connected ? STATE_ONLINE : STATE_OFFLINE;
    
    // Detect transition from ONLINE to OFFLINE
    if (sm->previous_state == STATE_ONLINE && sm->current_state == STATE_OFFLINE) {
        sm->outage_start = time(NULL);
        sm->outage_detected = false;
    }
    
    // Detect transition from OFFLINE to ONLINE
    if (sm->previous_state == STATE_OFFLINE && sm->current_state == STATE_ONLINE) {
        sm->outage_end = time(NULL);
        sm->outage_detected = true;
    }
}

bool is_state_changed(StateMachine* sm) {
    if (sm == NULL) return false;
    return sm->previous_state != sm->current_state;
}

long get_outage_duration(StateMachine* sm) {
    if (sm == NULL || sm->outage_end == 0 || sm->outage_start == 0) {
        return 0;
    }
    return (long)difftime(sm->outage_end, sm->outage_start);
}
