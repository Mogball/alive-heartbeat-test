#ifndef __INIT_INFO_H__
#define __INIT_INFO_H__

#include <vector>
#include <cstdint>

class InitInfo {
public:
    explicit InitInfo(std::vector<uint16_t> nodeIds);

private:
    std::vector<uint16_t> m_nodeIds;
};

#endif
