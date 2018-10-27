#include "message_handler.h"
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

static MessageQueue s_queue(InitInfo({10, 11, 12}));

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
            s_queue.pushMessage(packet);
        }
    }
    return nullptr;
}

static void *poll_func(void *) {
    s_queue.setHandler(4, &attendanceHandler);
    for (;;) {
        s_queue.handleNext();
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
