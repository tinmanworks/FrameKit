#pragma once
#include "NetInit.h"
#include "DatagramChannel.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace FrameKit::Net {

class NetManager {
public:
    NetManager();
    ~NetManager();

    NetManager(const NetManager&) = delete;
    NetManager& operator=(const NetManager&) = delete;

    // Create or fetch an existing channel by name
    DatagramChannel& CreateChannel(const std::string& name);

    // Convenience: create + bind
    DatagramChannel& CreateAndBind(const std::string& name, uint16_t port);

    // Call every tick to service receive queues
    void Tick();

private:
    std::unique_ptr<NetInit> m_Init;
    std::unordered_map<std::string, std::unique_ptr<DatagramChannel>> m_Channels;
};

} // namespace FrameKit::Net
