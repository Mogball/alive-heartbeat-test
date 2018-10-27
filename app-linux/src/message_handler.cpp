#include "message_handler.h"
#include <utility>

extern void nullMessageHandler(const wlp::Packet &);

MessageQueue::MessageQueue(InitInfo initInfo) :
        m_initInfo(std::move(initInfo)) {
    for (size_t i = 0; i < m_numHandlers; ++i) {
        m_handlers[i] = &nullMessageHandler;
    }
}

void MessageQueue::setHandler(uint32_t type, MessageHandler handler) {
    if (type < m_numHandlers) {
        m_handlers[type] = handler;
    }
}

void MessageQueue::pushMessage(const wlp::Packet &msg) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_messages.push(msg);
}

bool MessageQueue::handleNext() {
    bool hasMessage = !m_messages.empty();
    if (hasMessage) {
        const wlp::Packet &msg = m_messages.front();
        if (msg.type() < m_numHandlers) {
            m_handlers[msg.type()](msg);
        } else {
            nullMessageHandler(msg);
        }
        std::lock_guard<std::mutex> guard(m_mutex);
        m_messages.pop();
    }
    return hasMessage;
}
