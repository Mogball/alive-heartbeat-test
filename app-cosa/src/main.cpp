#include <MCP2515.h>
#include <CanPacket.h>
#include <CanPacketDriver.h>
#include <Cosa/MCP2515.h>
#include <Cosa/UART.hh>
#include <stdio.h>

#ifndef NODE_ID
#error "Must define NODE_ID"
#endif

#define uprintf(...) { \
    int __wrt = sprintf(s_buf, __VA_ARGS__); \
    uart.write(s_buf, __wrt); }

using namespace wlp;

static cosa::MCP2515 s_canBase(Board::D10);
static MCP2515 s_canBus(&s_canBase);
static Packet s_packet(NODE_ID, (uint32_t) 0, 0u);
static char s_buf[64];

void setup() {
    uart.begin(9600);
    while (Result::OK != s_canBus.begin(CAN_500KBPS, MCP_16MHz)) {
        uprintf("Failed to init CAN -> retrying\n");
        delay(100);
    }
    uprintf("CAN Inited successfully\n");
}

enum {
    IM_ALIVE = 0xffff00ff,

    ATTENDANCE = 0x01,
    HEARTBEAT  = 0x02
};

void loop() {
    delay(500);
    s_packet.type() = ATTENDANCE;
    s_packet.data() = IM_ALIVE;
    packet::send(s_canBus, s_packet);
}
