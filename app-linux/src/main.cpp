#include <MCP2515.h>
#include <CanPacket.h>
#include <CanPacketDriver.h>
#include <sys/mcp2515.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

using namespace wlp;

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
            printf("Id: %u\n", packet.id());
            printf("Type: %u\n", packet.type());
            printf("Data: %08x\n", packet.data());
        }
    }
}

int main(void) {
    pthread_t can_thread;
    int ret = pthread_create(&can_thread, nullptr, can_func, nullptr);
    if (0 != ret) {
        fprintf(stderr, "Error: pthread_create() -> %d\n", ret);
        return -1;
    }

    pthread_join(can_thread, nullptr);

    return 0;
}
