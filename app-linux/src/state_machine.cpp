#include "state_machine.h"
#include <climits>
#include <cstdio>

constexpr uint32_t s_allAttendance =
    (1 << 10) |
    (1 << 11) |
    (1 << 12);

MessageSM::MessageSM(MessageOut *out) :
    wlp::state_machine(ST_MAX_STATES, ST_ATTENDANCE),
    m_out(out) {}

void MessageSM::tick() {
    MessageData data;
    BEGIN_TRANSITION_MAP()
        TRANSITION_MAP_ENTRY(ST_POWERUP)          // ST_POWERUP
        TRANSITION_MAP_ENTRY(ST_ATTENDANCE)       // ST_ATTENDANCE
        TRANSITION_MAP_ENTRY(ST_ENABLE_HEARTBEAT) // ST_ENABLE_HEARTBEAT
        TRANSITION_MAP_ENTRY(ST_IDLE)             // ST_IDLE
        TRANSITION_MAP_ENTRY(ST_ERROR)            // ST_ERROR
    END_TRANSITION_MAP(&data, MessageData);
}

void MessageSM::begin() {
    MessageData data;
    BEGIN_TRANSITION_MAP()
        TRANSITION_MAP_ENTRY(ST_ATTENDANCE) // ST_POWERUP
        TRANSITION_MAP_ENTRY(ST_ERROR)      // ST_ATTENDANCE
        TRANSITION_MAP_ENTRY(ST_ERROR)      // ST_ENABLE_HEARTBEAT
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
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)       // ST_POWERUP
        TRANSITION_MAP_ENTRY(ST_ENABLE_HEARTBEAT) // ST_ATTENDANCE
        TRANSITION_MAP_ENTRY(ST_IDLE)             // ST_ENABLE_HEARTBEAT
        TRANSITION_MAP_ENTRY(ST_ERROR)            // ST_IDLE
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)       // ST_ERROR
    END_TRANSITION_MAP(&data, MessageData)
}

void MessageSM::registerHeartbeat(uint16_t id) {
}

STATE_DEFINE(MessageSM, StPowerup, MessageData) {
}

STATE_DEFINE(MessageSM, StAttendance, MessageData) {
}

STATE_DEFINE(MessageSM, StEnableHeartbeat, MessageData) {
    m_out->emplaceMessage(0xff, 0xffff00ff, 0x02);
}

STATE_DEFINE(MessageSM, StIdle, MessageData) {
    m_out->emplaceMessage(0xff, 0xffff00ff, 0x02);
}

STATE_DEFINE(MessageSM, StError, MessageData) {
}

GUARD_DEFINE(MessageSM, GdIdle, MessageData) {
    return s_allAttendance == m_attendance;
}

EXIT_DEFINE(MessageSM, ExAttendance) {
    m_attendance = 0U;
}
