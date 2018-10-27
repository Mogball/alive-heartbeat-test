#include "init_info.h"
#include <utility>

InitInfo::InitInfo(std::vector<uint16_t> nodeIds) :
    m_nodeIds(std::move(nodeIds)) {}
