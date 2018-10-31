#include "message_out.h"

void MessageOut::emplaceMessage(uint32_t id, uint32_t data, uint32_t type) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_messages.emplace(id, data, type);
}

void MessageOut::pushMessage(const wlp::Packet &msg) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_messages.push(msg);
}

bool MessageOut::hasNext() {
    std::lock_guard<std::mutex> guard(m_mutex);
    return !m_messages.empty();
}

wlp::Packet MessageOut::getNext() {
    std::lock_guard<std::mutex> guard(m_mutex);
    auto ret = m_messages.front();
    return ret;
}
