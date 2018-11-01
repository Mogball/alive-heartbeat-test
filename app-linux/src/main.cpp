#include "message_handler.h"
#include "message_out.h"
#include "state_machine.h"
#include <MCP2515.h>
#include <CanPacket.h>
#include <CanPacketDriver.h>
#include <sys/mcp2515.h>
#include <wlib/timer>
#include <unistd.h>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>

using namespace wlp;

static MessageOut s_in;
static MessageOut s_out;

static bool ready(MCP2515 &bus) {
    return MessageState::MessagePending == bus.get_message_status();
}

static void *can_func(void *) {
    linux::MCP2515 canBase("/dev/spidev0.0", 10 * 1000 * 1000);
    MCP2515 canBus(&canBase);
    Packet packet;
    canBase.begin();

    while (Result::OK != canBus.begin(CAN_500KBPS, MCP_8MHz)) {
        puts("CAN Init failed -> retrying");
        sleep(1);
    }
    puts("CAN Init success");

    for (;;) {
        if (ready(canBus)) {
            packet::read(canBus, packet);
            s_in.pushMessage(packet);
        }
        if (s_out.hasNext()) {
            packet = s_out.getNext();
            packet::send(canBus, packet);
        }
        pthread_yield();
    }
    return nullptr;
}

struct zero_timer {
    zero_timer() {
        t.zero();
    }
    timer t;
};

typedef void (*fhandler_t)(Packet &);
typedef void (*fstate_t)();

static char s_msgBuf[256];
static size_t s_state;
static fhandler_t s_handlers[256];
static fstate_t s_stateFuncs[256];
static uint32_t s_attendance;
constexpr uint32_t s_expectedAttendance = (7 << 10);

static void fatalError(const char *msg) {
    for (;;) {
        puts(msg);
        sleep(500);
    }
}

static void nothing() {}
static void ignored(Packet &p) {}

static void unhandled(Packet &p) {
    printf("WARNING: Unhandled message type %d, %d, %08x\n", (int) p.type(), (int) p.id(), p.data());
}

static void takeAttendance(Packet &p) {
    uint16_t id = p.id();
    uint32_t prev = s_attendance;
    s_attendance |= (1 << id);
    if (s_attendance != prev) {
        printf("INFO: Node attended: %d\n", (int) id);
    }
    if ((s_attendance | s_expectedAttendance) != s_expectedAttendance) {
        sprintf(s_msgBuf, "ERROR: Unexpected attendance ID: %d\n", (int) id);
        fatalError(s_msgBuf);
    }

    if (s_attendance == s_expectedAttendance) {
        printf("INFO: All nodes reporting\n");
        s_attendance = 0;
        s_handlers[1] = &ignored;
        s_handlers[2] = &takeAttendance;
        s_state = 1;
    }
}

static void sendHeartbeat() {
    static zero_timer timer;
    if (timer.t.ms() <= 500) {
        return;
    }
    printf("INFO: Sending heartbeat\n");
    s_out.emplaceMessage(0xff, 0xffff00ff, 0x02);
    timer.t.zero();
}

static void *poll_func(void *) {
    for (size_t i = 0; i < sizeof(s_handlers) / sizeof(s_handlers[0]); ++i) {
        s_handlers[i] = &unhandled;
        s_stateFuncs[i] = &nothing;
    }
    s_handlers[1] = &takeAttendance;
    s_stateFuncs[1] = &sendHeartbeat;

    Packet packet;
    for (;;) {
        if (s_in.hasNext()) {
            packet = s_in.getNext();
            (*s_handlers[packet.type()])(packet);
        }
        (*s_stateFuncs[s_state])();
        pthread_yield();
    }
    return nullptr;
}

int main(void) {
    pthread_t can_thread;
    pthread_t poll_thread;
    int ret = pthread_create(&can_thread, nullptr, can_func, nullptr);
    if (0 != ret) {
        fprintf(stderr, "Error: pthread_create() -> %d\n", ret);
        return -1;
    }
    ret = pthread_create(&poll_thread, nullptr, poll_func, nullptr);
    if (0 != ret) {
        fprintf(stderr, "Error: pthread_create() -> %d\n", ret);
        return -1;
    }

    pthread_join(can_thread, nullptr);
    pthread_join(poll_thread, nullptr);

    return 0;
}
