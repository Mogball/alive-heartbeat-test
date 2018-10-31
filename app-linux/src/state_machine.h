#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "message_out.h"
#include <CanPacket.h>
#include <wlib/state_machine>
#include <cstdint>

class MessageData : public wlp::sm_event_data {};

class MessageSM : public wlp::state_machine {
public:
    enum States {
        ST_POWERUP,
        ST_ATTENDANCE,
        ST_ENABLE_HEARTBEAT,
        ST_IDLE,
        ST_ERROR,

        ST_MAX_STATES
    };

    MessageSM(MessageOut *out);

    void tick();
    void begin();
    void takeAttendance(uint16_t id);
    void registerHeartbeat(uint16_t id);

private:
    uint32_t m_attendance;
    MessageOut *m_out;

    STATE_DECLARE(MessageSM, StPowerup, MessageData);
    STATE_DECLARE(MessageSM, StAttendance, MessageData);
    STATE_DECLARE(MessageSM, StEnableHeartbeat, MessageData);
    STATE_DECLARE(MessageSM, StIdle, MessageData);
    STATE_DECLARE(MessageSM, StError, MessageData);

    GUARD_DECLARE(MessageSM, GdIdle, MessageData);
    EXIT_DECLARE(MessageSM, ExAttendance);

    BEGIN_STATE_MAP_EX()
        STATE_MAP_ENTRY_EX(&StPowerup)
        STATE_MAP_ENTRY_ALL_EX(&StAttendance, nullptr, nullptr, &ExAttendance)
        STATE_MAP_ENTRY_ALL_EX(&StEnableHeartbeat, &GdIdle, nullptr, nullptr)
        STATE_MAP_ENTRY_ALL_EX(&StIdle, &GdIdle, nullptr, nullptr)
        STATE_MAP_ENTRY_EX(&StError)
    END_STATE_MAP_EX()
};

extern void tickHandler();

extern void powerupHandler(const wlp::Packet &msg);
extern void attendanceHandler(const wlp::Packet &msg);

#endif
