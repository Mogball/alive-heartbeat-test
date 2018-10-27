#include "state_machine.h"

constexpr uint32_t s_allAttendance =
    (1 << 10) |
    (1 << 11) |
    (1 << 12);

MessageSM::MessageSM() :
    wlp::state_machine(ST_MAX_STATES, ST_POWERUP) {}

void MessageSM::begin() {
    MessageData data;
    BEGIN_TRANSITION_MAP()
        TRANSITION_MAP_ENTRY(ST_ATTENDANCE) // ST_POWERUP
        TRANSITION_MAP_ENTRY(ST_ERROR)      // ST_ATTENDANCE
        TRANSITION_MAP_ENTRY(ST_ERROR)      // ST_IDLE
        TRANSITION_MAP_ENTRY(EVENT_IGNORED) // ST_ERROR
    END_TRANSITION_MAP(&data, MessageData)
}

void MessageSM::takeAttendance(uint16_t id) {
    if (id < sizeof(uint32_t) * CHAR_BIT) {
        m_attendance |= (1 << id);
    }
    MessageData data;
    BEGIN_TRANSITION_MAP()
        TRANSITION_MAP_ENTRY(EVENT_IGNORED) // ST_POWERUP
        TRANSITION_MAP_ENTRY(ST_IDLE)       // ST_ATTENDANCE
        TRANSITION_MAP_ENTRY(ST_ERROR)      // ST_IDLE
        TRANSITION_MAP_ENTRY(EVENT_IGNORED) // ST_ERROR
    END_TRANSITION_MAP(&data, MessageData)
}

STATE_DEFINE(MessageSM, StPowerup, MessageData) {
}

STATE_DEFINE(MessageSM, StAttendance, MessageData) {
}

STATE_DEFINE(MessageSM, StIdle, MessageData) {
}

STATE_DEFINE(MessageSM, StError, MessageData) {
}

GUARD_DEFINE(MessageSM, GdIdle, MessageData) {
}
