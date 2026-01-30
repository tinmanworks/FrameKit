#pragma once
#include <cstdint>
#include <string>

namespace FrameKit::Net {

struct Endpoint {
    std::string ip;
    uint16_t port = 0;

    Endpoint() = default;
    Endpoint(std::string ip_, uint16_t port_) : ip(std::move(ip_)), port(port_) {}

    bool IsValid() const { return !ip.empty() && port != 0; }
};

} // namespace FrameKit::Net
