#include "state_machine.h"

static MessageOut s_out;
static MessageSM s_sm(&s_out);

void tickHandler() {
    s_sm.tick();
}

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
