// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Events/GlobalEventHandler.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Global event handler
// =============================================================================

#pragma once

#include "FrameKit/Events/Event.h"

#include <functional>
#include <mutex>
#include <vector>

namespace FrameKit {

// Simple global event handler singleton
class GlobalEventHandler {
public:
    using Listener = std::function<void(Event&)>;

    static GlobalEventHandler& Get();

    void AddListener(const Listener& l);
    void Emit(Event& e);

private:
    GlobalEventHandler() = default;
    std::vector<Listener> m_Listeners;
    std::mutex m_Mutex;
};

} // namespace FrameKit
