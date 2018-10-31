#include "message_handler.h"
#include "message_out.h"
#include "state_machine.h"
#include <MCP2515.h>
#include <CanPacket.h>
#include <CanPacketDriver.h>
#include <sys/mcp2515.h>
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
    }
    return nullptr;
}

typedef void (*fhandler_t)(Packet &);

static char s_msgBuf[256];
static size_t s_state;
static fhandler_t s_handlers[256];
static uint32_t s_attendance;
constexpr uint32_t s_expectedAttendance = (7 << 10);

static void fatalError(const char *msg) {
    for (;;) {
        puts(msg);
        sleep(500);
    }
}

static void unhandled(Packet &p) {
    printf("WARNING: Unhandled message type %d\n", (int) p.type());
}

static void takeAttendance(Packet &p) {
    uint16_t id = p.id();
    s_attendance |= (1 << id);
    if (s_attendance | s_expectedAttendance != s_expectedAttendance) {
        sprintf(s_msgBuf, "ERROR: Unexpected attendance ID: %d\n", (int) id);
        fatalError(s_msgBuf);
    }

    if (s_attendance == s_expectedAttendance) {
        printf("All nodes reporting\n");
    }
}

static void *poll_func(void *) {
    for (size_t i = 0; i < sizeof(s_handlers) / sizeof(s_handlers[0]); ++i) {
        s_handlers[i] = &unhandled;
    }
    s_handlers[2] = &takeAttendance;

    Packet packet;
    for (;;) {
        if (s_in.hasNext()) {
            packet = s_in.getNext();
            (*s_handlers[packet.type()])(packet);
        }
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
