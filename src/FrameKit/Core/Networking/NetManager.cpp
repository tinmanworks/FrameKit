#include "FrameKit/Networking/NetManager.h"

namespace FrameKit::Net {

NetManager::NetManager() : m_Init(std::make_unique<NetInit>()) {}
NetManager::~NetManager() = default;

DatagramChannel& NetManager::CreateChannel(const std::string& name) {
    auto it = m_Channels.find(name);
    if (it != m_Channels.end()) return *it->second;

    auto ch = std::make_unique<DatagramChannel>(name);
    ch->Open();

    auto* ptr = ch.get();
    m_Channels.emplace(name, std::move(ch));
    return *ptr;
}

DatagramChannel& NetManager::CreateAndBind(const std::string& name, uint16_t port) {
    auto& ch = CreateChannel(name);
    ch.Bind(port);
    return ch;
}

void NetManager::Tick() {
    for (auto& [_, ch] : m_Channels) {
        ch->Poll();
    }
}

} // namespace FrameKit::Net
