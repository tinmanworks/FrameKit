#pragma once
#include <stdexcept>

namespace FrameKit::Net {

class NetInit {
public:
    NetInit();
    ~NetInit();

    NetInit(const NetInit&) = delete;
    NetInit& operator=(const NetInit&) = delete;

    static bool IsInitialized();

private:
    static inline bool s_inited = false;
};

} // namespace FrameKit::Net
