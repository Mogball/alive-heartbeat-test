#ifndef __MESSAGE_OUT_H__
#define __MESSAGE_OUT_H__

#include <CanPacket.h>
#include <queue>
#include <mutex>

class MessageOut {
public:
    void emplaceMessage(uint32_t id, uint32_t data, uint32_t type);
    void pushMessage(const wlp::Packet &msg);
    bool hasNext();
    wlp::Packet getNext();

private:
    std::mutex m_mutex;
    std::queue<wlp::Packet> m_messages;
};

#endif
