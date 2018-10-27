#include "state_machine.h"

static MessageSM s_sm;

void powerupHandler(const wlp::Packet &msg) {
    if (msg.data() == 0xffff000f) {
        s_sm.begin();
    }
}

void attendanceHandler(const wlp::Packet &msg) {
    if (msg.data() == 0xffff00ff) {
        s_sm.takeAttendance(msg.id());
    }
}
