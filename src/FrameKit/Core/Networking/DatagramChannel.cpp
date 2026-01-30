#include "FrameKit/Networking/DatagramChannel.h"

namespace FrameKit::Net {

DatagramChannel::DatagramChannel(std::string name)
    : m_Name(std::move(name)) {}

bool DatagramChannel::Open() { return m_Sock.Open(); }
void DatagramChannel::Close() { m_Sock.Close(); }

bool DatagramChannel::Bind(uint16_t port) { return m_Sock.Bind(port); }

void DatagramChannel::SetPeer(Endpoint peer) { m_Peer = std::move(peer); }

bool DatagramChannel::EnableBroadcast(bool enabled) {
    return m_Sock.SetBroadcast(enabled);
}

void DatagramChannel::On(uint8_t type, Handler cb) {
    m_Handlers[type] = std::move(cb);
}

bool DatagramChannel::Send(uint8_t type, std::span<const uint8_t> payload) {
    if (!m_Peer.IsValid()) return false;
    return SendTo(m_Peer, type, payload);
}

bool DatagramChannel::SendTo(const Endpoint& to, uint8_t type, std::span<const uint8_t> payload) {
    auto bytes = Encode(type, m_SeqTx++, payload);
    return m_Sock.SendTo(to, bytes);
}

void DatagramChannel::Poll() {
    // Drain all available packets
    for (;;) {
        auto pkt = m_Sock.RecvOnce(4096);
        if (!pkt) break;

        DecodedPacket dec{};
        if (!Decode(pkt->data, dec)) continue;

        auto it = m_Handlers.find(dec.type);
        if (it != m_Handlers.end() && it->second) {
            it->second(pkt->from, dec);
        }
    }
}

} // namespace FrameKit::Net
