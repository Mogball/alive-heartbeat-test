#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include <CanPacket.h>
#include <wlib/state_machine>
#include <cstdint>

class MessageData : public wlp::sm_event_data {};

class MessageSM : public wlp::state_machine {
public:
    enum States {
        ST_POWERUP,
        ST_ATTENDANCE,
        ST_IDLE,
        ST_ERROR,

        ST_MAX_STATES
    };

    MessageSM();

    void begin();
    void takeAttendance(uint16_t id);

private:
    uint32_t m_attendance;

    STATE_DECLARE(MessageSM, StPowerup, MessageData);
    STATE_DECLARE(MessageSM, StAttendance, MessageData);
    STATE_DECLARE(MessageSM, StIdle, MessageData);
    STATE_DECLARE(MessageSM, StError, MessageData);

    GUARD_DECLARE(MessageSM, GdIdle, MessageData);

    BEGIN_STATE_MAP_EX()
        STATE_MAP_ENTRY_EX(&StPowerup)
        STATE_MAP_ENTRY_EX(&StAttendance)
        STATE_MAP_ENTRY_ALL_EX(&StIdle, &GdIdle, nullptr, nullptr)
        STATE_MAP_ENTRY_EX(&StError)
    END_STATE_MAP_EX()
};

#endif
