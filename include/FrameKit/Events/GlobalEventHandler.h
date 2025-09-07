// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/GlobalEventHandler.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Singleton for routing global events. Provides a single, thread-safe
//      callback slot you can set/clear and an Emit() helper to dispatch.
// =============================================================================

#pragma once

#include "FrameKit/Base/Base.h"
#include "FrameKit/Events/Event.h"

#include <functional>
#include <mutex>

namespace FrameKit {

    class GlobalEventHandler final {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        // Access singleton
        FK_NODISCARD static GlobalEventHandler& Get() noexcept;

        // Install/replace the callback (thread-safe)
        void SetEventCallback(EventCallbackFn callback);

        // Remove any installed callback (thread-safe)
        void ClearEventCallback();

        // Dispatch one event to the installed callback if present (thread-safe)
        void Emit(Event& e);

        // Hook for per-frame work if needed later (currently no-op)
        void OnUpdate() noexcept;

        // Non-owning access to check if a callback is installed (thread-safe)
        bool HasCallback() const noexcept;

        // Deleted copying/moving (singleton)
        GlobalEventHandler(const GlobalEventHandler&) = delete;
        GlobalEventHandler(GlobalEventHandler&&) = delete;
        GlobalEventHandler& operator=(const GlobalEventHandler&) = delete;
        GlobalEventHandler& operator=(GlobalEventHandler&&) = delete;

    private:
        GlobalEventHandler() = default;
        ~GlobalEventHandler() = default;

    private:
        mutable std::mutex m_Mutex;
        EventCallbackFn    m_Callback; // guarded by m_Mutex
    };

} // namespace FrameKit
