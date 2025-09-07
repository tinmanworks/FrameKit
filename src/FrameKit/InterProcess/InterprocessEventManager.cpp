/*
 * Project: FrameKit
 * File: InterprocessEventManager.cpp
 * Author: George Gil
 * Date: 2025-08-22
 * License: MIT
 * Description:
 *   Interprocess event queue backed by shared memory and an SPSC ring.
 *   Master pushes events; slave polls and dispatches registered callbacks.
 */

#include "FrameKit/InterProcess/InterprocessEventManager.h"
#include <type_traits>

namespace FrameKit::IPC {

InterprocessEventManager::InterprocessEventManager(const char* shmName) {
    // Create or open a typed mapping that holds exactly Ring; constructor runs once (creator).
    m_Ring = FrameKit::IPC::CreateTypedSharedMemory<Ring>(
        shmName,
        FrameKit::IPC::OpenMode::OpenOrCreate,
        &m_Mapping
    );
    // If m_Ring is null, mapping failed; add logging if desired.
}

InterprocessEventManager::~InterprocessEventManager() noexcept {
    if (m_Mapping.base) {
        FrameKit::IPC::CloseSharedMemory(m_Mapping);
    }
}

void InterprocessEventManager::PushDescriptor(const InterprocessEvent& e) noexcept {
    if (!m_Ring) return;
    // Non-blocking; if full, we drop newest (control-plane semantics).
    (void)m_Ring->try_push(e);
}

void InterprocessEventManager::CheckSharedMemory() noexcept {
    if (!m_Ring) return;

    InterprocessEvent ev;
    while (m_Ring->try_pop(ev)) {
        auto it = m_Map.find(ev.key);
        if (it == m_Map.end()) continue;

        auto& v = it->second.cb;
        std::visit([&](auto&& cb){
            using T = std::decay_t<decltype(cb)>;
            if constexpr (std::is_same_v<T, std::function<void()>>) {
                cb();
            } else if constexpr (std::is_same_v<T, std::function<void(const uint8_t*)>>) {
                cb(ev.payload);
            }
        }, v);
    }
}

} // namespace FrameKit::IPC
