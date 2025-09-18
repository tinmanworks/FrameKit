// =============================================================================
// Project: FrameKit
// File         : include/FrameKit/Window.h
// Author       : George Gil
// Created      : 2025-09-18
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		Central include for windowing system. Includes window backends
//		and provides utility functions to access windows.
// =============================================================================

#pragma once

#include "FrameKit/Window/WindowRegistry.h"
#include "FrameKit/Engine/Defines.h"

#include <vector>


namespace FrameKit {
#if defined(FK_WINDOW_BACKEND_WIN32_AVAIL)
#if defined(FK_WINDOW_BACKEND_WIN32_ENABLE)
	extern "C" bool FrameKit_RegisterBackend_Win32();
#endif
#endif
#if defined(FK_WINDOW_BACKEND_GLFW_AVAIL)
#if defined(FK_WINDOW_BACKEND_GLFW_ENABLE)
	extern "C" bool FrameKit_RegisterBackend_GLFW();
#endif
#endif
#if defined(FK_WINDOW_BACKEND_COCOA_AVAIL)
#if defined(FK_WINDOW_BACKEND_COCOA_ENABLE)
	extern "C" bool FrameKit_RegisterBackend_Cocoa();
#endif
#endif

// Register compiled-in backends once. Returns number registered.
inline int InitializeWindowBackends() noexcept {
    static bool ran = false;
    static int count = 0;
    if (ran) return count;
    ran = true;
	
#if defined(FK_WINDOW_BACKEND_WIN32_AVAIL) && defined(FK_WINDOW_BACKEND_WIN32_ENABLE)
    if (FrameKit_RegisterBackend_Win32()) ++count;
#endif
#if defined(FK_WINDOW_BACKEND_GLFW_AVAIL) && defined(FK_WINDOW_BACKEND_GLFW_ENABLE)
    if (FrameKit_RegisterBackend_GLFW()) ++count;
#endif
#if defined(FK_WINDOW_BACKEND_COCOA_AVAIL) && defined(FK_WINDOW_BACKEND_COCOA_ENABLE)
    if (FrameKit_RegisterBackend_Cocoa()) ++count;
#endif
    return count;
	}

	
	// Convenience accessors over the registry.
	// Non-owning pointers. Snapshot semantics.

	FK_NODISCARD inline std::vector<WindowInfo> ListWindows() {
		return WindowRegistry::List();
	}

	FK_NODISCARD inline IWindow* GetWindowById(WindowId id) noexcept {
		return WindowRegistry::Get(id);
	}

	// “Main” = first in the current snapshot. Caller may define stricter policy.
	FK_NODISCARD inline WindowInfo GetPrimaryWindowInfo() {
		auto v = WindowRegistry::List();
		return v.empty() ? WindowInfo{} : v.front();
	}

	FK_NODISCARD inline IWindow* GetPrimaryWindow() noexcept {
		auto v = WindowRegistry::List();
		return v.empty() ? nullptr : WindowRegistry::Get(v.front().id);
	}

	FK_NODISCARD inline WindowId GetPrimaryWindowId() noexcept {
		auto v = WindowRegistry::List();
		return v.empty() ? WindowId{0} : v.front().id;
	}
} // namespace FrameKit