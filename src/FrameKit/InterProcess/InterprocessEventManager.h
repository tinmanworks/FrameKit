/*
 * Project: FrameKit
 * File: InterprocessEventManager.h
 * Author: George Gil
 * Date: 2025-08-22
 * License: MIT
 * Description:
 *   Interprocess event manager using SharedMemory + an SPSC ring.
 *   Master pushes InterprocessEvent records; slave polls and dispatches
 *   to callbacks registered by deterministic 31× rolling keys.
 */

#pragma once
#include <unordered_map>
#include <variant>
#include <functional>
#include <string>
#include <utility> // for std::move

#include "FrameKit/SharedMemory/SharedMemory.h"
#include "FrameKit/InterProcess/InterprocessRing.hpp"
#include "FrameKit/InterProcess/InterprocessEvent.hpp"

namespace FrameKit::IPC {

using CallbackVariant = std::variant<
    std::function<void()>,
    std::function<void(const uint8_t*)>
>;

struct CallbackEntry {
    uint32_t        key;
    CallbackVariant cb;
};

class InterprocessEventManager {
public:
    // One well-known queue name by default; override if you need per-master isolation.
    explicit InterprocessEventManager(const char* shmName = "FrameKit.IPC.EventQueue");
    ~InterprocessEventManager() noexcept;

    InterprocessEventManager(const InterprocessEventManager&) = delete;
    InterprocessEventManager& operator=(const InterprocessEventManager&) = delete;
    InterprocessEventManager(InterprocessEventManager&&) = delete;
    InterprocessEventManager& operator=(InterprocessEventManager&&) = delete;

    // Sender (master)
    void PushDescriptor(const InterprocessEvent& e) noexcept;

    // Receiver (slave): non-blocking, drain and dispatch
    void CheckSharedMemory() noexcept;

    template <typename CB>
    void RegisterCallback(uint32_t key, CB cb) {
        m_Map[key] = CallbackEntry{ key, CallbackVariant(std::move(cb)) };
    }

private:
    // Power-of-two ring capacity; 64 is a good control-plane default.
    using Ring = SpscRing<InterprocessEvent, 64>;

    FrameKit::IPC::SharedMemory                    m_Mapping{};
    Ring*                                          m_Ring = nullptr;
    std::unordered_map<uint32_t, CallbackEntry>    m_Map;
};

} // namespace FrameKit::IPC
