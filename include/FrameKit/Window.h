// =============================================================================
// Project: FrameKit
// File         : include/FrameKit/Window.h
// Author       : George Gil
// Created      : 2025-09-18
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//   Engine-owned entry. Delegates to client CreateApplication and runs the host loop.
// =============================================================================

#pragma once

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

	void InitializeWindowBackends() {
#if defined(FK_WINDOW_BACKEND_WIN32_AVAIL)
#if defined(FK_WINDOW_BACKEND_WIN32_ENABLE)
		(void)FrameKit_RegisterBackend_Win32();
#endif
#endif

#if defined(FK_WINDOW_BACKEND_GLFW_AVAIL)
#if defined(FK_WINDOW_BACKEND_GLFW_ENABLE)
		(void)FrameKit_RegisterBackend_GLFW();
#endif
#endif

#if defined(FK_WINDOW_BACKEND_COCOA_AVAIL)
#if defined(FK_WINDOW_BACKEND_COCOA_ENABLE)
		(void)FrameKit_RegisterBackend_Cocoa();
#endif
#endif
	}
}