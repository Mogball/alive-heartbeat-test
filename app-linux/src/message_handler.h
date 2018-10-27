#ifndef __MESSAGE_HANDLER_H__
#define __MESSAGE_HANDLER_H__

#include "init_info.h"
#include <CanPacket.h>
#include <queue>
#include <mutex>
#include <cstdint>

typedef void (*MessageHandler)(const wlp::Packet &);

class MessageQueue {
    static constexpr size_t m_numHandlers = 1 << 8;
public:
    explicit MessageQueue(InitInfo initInfo);

    void setHandler(uint32_t type,  MessageHandler handler);
    void pushMessage(const wlp::Packet &msg);
    bool handleNext();

private:
    InitInfo m_initInfo;
    std::mutex m_mutex;
    std::queue<wlp::Packet> m_messages;
    MessageHandler m_handlers[m_numHandlers];
};

#endif
