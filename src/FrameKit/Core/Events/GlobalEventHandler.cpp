// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Events/GlobalEventHandler.cpp
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Global event handler
// =============================================================================

#include "FrameKit/Events/GlobalEventHandler.h"

namespace FrameKit {

GlobalEventHandler& GlobalEventHandler::Get() {
    static GlobalEventHandler inst;
    return inst;
}

void GlobalEventHandler::AddListener(const Listener& l) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Listeners.push_back(l);
}

void GlobalEventHandler::Emit(Event& e) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& l : m_Listeners) {
        l(e);
        if (e.Handled) break;
    }
}

} // namespace FrameKit
