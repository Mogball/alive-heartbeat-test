#include <MCP2515.h>
#include <CanPacket.h>
#include <CanPacketDriver.h>
#include <Cosa/MCP2515.h>
#include <Cosa/UART.hh>
#include <stdint.h>
#include <stdio.h>
#include <wlib/queue>
#include <wlib/timer>

#ifndef NODE_ID
#error "Must define NODE_ID"
#endif

#define uprintf(...) { \
    int __wrt = sprintf(s_buf, __VA_ARGS__); \
    uart.write(s_buf, __wrt); }

using namespace wlp;

struct zero_timer {
    zero_timer() { t.zero(); }
    timer t;
};

static cosa::MCP2515 s_canBase(Board::D10);
static MCP2515 s_canBus(&s_canBase);
static char s_buf[64];

static bool ready(MCP2515 &bus) {
    return MessageState::MessagePending == bus.get_message_status();
}

void setup() {
    timer::begin();
    uart.begin(9600);
    while (Result::OK != s_canBus.begin(CAN_500KBPS, MCP_16MHz)) {
        uprintf("ERROR: Failed to init CAN -> retrying\n");
        delay(100);
    }
    uprintf("INFO: CAN Inited successfully\n");
}

enum {
    IM_ALIVE = 0xffff00ff,

    ATTENDANCE = 0x01,
    HEARTBEAT  = 0x02
};

static queue<Packet, 32> s_in;
static queue<Packet, 32> s_out;

static void serviceCan() {
    static Packet msg;
    if (ready(s_canBus)) {
        packet::read(s_canBus, msg);
        s_in.enqueue(msg);
    }
    while (!s_out.empty()) {
        msg = s_out.peek();
        s_out.dequeue();
        packet::send(s_canBus, msg);
    }
}

typedef void (*fstate_t)();
static void stateAttendance();

static uint32_t s_state;
static fstate_t s_stateFuncs[32] = {
    &stateAttendance
};
static Packet s_nodeMsg(NODE_ID, IM_ALIVE, ATTENDANCE);

static void stateAttendance() {
    static zero_timer aliveTimer;
    if (aliveTimer.t.ms() > 100) {
        aliveTimer.t.zero();
        s_out.enqueue(s_nodeMsg);
    }
}

static void serviceState() {
    (*s_stateFuncs[s_state])();
}

void loop() {
    serviceCan();
    serviceState();
}
