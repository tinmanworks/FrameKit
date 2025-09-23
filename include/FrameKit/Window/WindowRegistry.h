// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Window/WindowRegistry.h
// Author       : George Gil
// Created      : 2025-09-18
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Window registry for managing window instances.
// =============================================================================

#pragma once

#include "FrameKit/Window/IWindow.h"

namespace FrameKit {
    using WindowId = std::uint64_t;
    struct WindowInfo { WindowId id; IWindow* ptr; WindowAPI api; std::string name; };

    class WindowRegistry {
    public:
        static WindowId Register(IWindow* w, WindowAPI api, std::string name);
        static void     Unregister(IWindow* w) noexcept;
        static IWindow* Get(WindowId id) noexcept;
        static std::vector<WindowInfo> List();
    };

    template<void(*DestroyFn)(IWindow*)>
    inline void DestroyAndUnregister(IWindow* w) noexcept {
        if (!w) return;
        WindowRegistry::Unregister(w);
        DestroyFn(w);
    }
}