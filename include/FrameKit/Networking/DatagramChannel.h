#pragma once
#include "UdpSocket.h"
#include "Packet.h"

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace FrameKit::Net {

class DatagramChannel {
public:
    using Handler = std::function<void(const Endpoint& from, const DecodedPacket& pkt)>;

public:
    explicit DatagramChannel(std::string name = "");

    bool Open();
    void Close();

    bool Bind(uint16_t port);      // receive on this port
    void SetPeer(Endpoint peer);   // default send destination
    const Endpoint& GetPeer() const { return m_Peer; }

    bool EnableBroadcast(bool enabled);

    // register handler for a message type
    void On(uint8_t type, Handler cb);

    // Send to configured peer
    bool Send(uint8_t type, std::span<const uint8_t> payload);

    // Send to explicit endpoint
    bool SendTo(const Endpoint& to, uint8_t type, std::span<const uint8_t> payload);

    // Drain receive queue and dispatch handlers (non-blocking)
    void Poll();

private:
    std::string m_Name;
    UdpSocket m_Sock;
    Endpoint m_Peer;

    uint32_t m_SeqTx = 0;

    std::unordered_map<uint8_t, Handler> m_Handlers;
};

} // namespace FrameKit::Net
