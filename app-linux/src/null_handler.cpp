#include <CanPacket.h>
#include <cstdio>

void nullMessageHandler(const wlp::Packet &msg) {
    printf("WARNING: Unhandled message type %d\n", msg.type());
    printf("ID: 0x%03x, Data: 0x%08x\n", msg.id(), msg.data());
}
