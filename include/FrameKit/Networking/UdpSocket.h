#pragma once
#include "Endpoint.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace FrameKit::Net {

class UdpSocket {
public:
    struct RecvPacket {
        Endpoint from;
        std::vector<uint8_t> data;
    };

public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    bool Open();
    void Close();

    bool IsOpen() const;

    // Bind to a local port (0 means ephemeral if supported by OS; best to use fixed ports)
    bool Bind(uint16_t port);

    // Enable/disable broadcast sending (needed for 255.255.255.255)
    bool SetBroadcast(bool enabled);

    bool SendTo(const Endpoint& to, std::span<const uint8_t> bytes);

    // Non-blocking: returns nullopt if no packet available
    std::optional<RecvPacket> RecvOnce(size_t maxBytes = 2048);

private:
    void SetNonBlocking();

private:
    void* m_Handle = nullptr; // opaque (SOCKET on Windows, int on POSIX)
};

} // namespace FrameKit::Net
