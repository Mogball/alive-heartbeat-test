#include <MCP2515.h>
#include <Cosa/MCP2515.h>
#include <Cosa/UART.hh>
#include <stdio.h>

#define uprintf(...) { \
    int __wrt = sprintf(s_buf, __VA_ARGS__); \
    uart.write(s_buf, __wrt); }

using namespace wlp;

static cosa::MCP2515 s_canBase(Board::D10);
static MCP2515 s_canBus(&s_canBase);
static char s_buf[64];

void setup() {
    uart.begin(9600);
    while (Result::OK != s_canBus.begin(CAN_500KBPS, MCP_16MHz)) {
        uprintf("Failed to init CAN -> retrying\n");
        delay(100);
    }
    uprintf("CAN Inited successfully\n");
}

void loop() {
    delay(500);
    uprintf("_loop_\n");
}
