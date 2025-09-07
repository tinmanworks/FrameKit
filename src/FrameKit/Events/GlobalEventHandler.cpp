// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Events/GlobalEventHandler.cpp
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Singleton for routing global events. Provides a single, thread-safe
//      callback slot you can set/clear and an Emit() helper to dispatch.
// =============================================================================

#include "FrameKit/Events/GlobalEventHandler.h"

namespace FrameKit {

    GlobalEventHandler& GlobalEventHandler::Get() noexcept {
        static GlobalEventHandler instance;
        return instance;
    }

    void GlobalEventHandler::SetEventCallback(EventCallbackFn callback) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Callback = std::move(callback);
    }

    void GlobalEventHandler::ClearEventCallback() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Callback = nullptr;
    }

    bool GlobalEventHandler::HasCallback() const noexcept {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return static_cast<bool>(m_Callback);
    }

    void GlobalEventHandler::Emit(Event& e) {
        EventCallbackFn cb;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            cb = m_Callback; // copy under lock, invoke outside
        }
        if (cb) {
            cb(e);
        }
    }

    void GlobalEventHandler::OnUpdate() noexcept {
        // No-op for now; placeholder for queued/async event pumping if you add it later.
    }

} // namespace FrameKit
