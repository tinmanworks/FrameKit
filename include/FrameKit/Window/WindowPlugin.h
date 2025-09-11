// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Window/WindowPlugin.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Window backend plugin interface
// =============================================================================

#pragma once

#include "FrameKit/Engine/PlatformDetection.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FRAMEKIT_WINDOW_PLUGIN_ABI 1

    typedef struct FrameKit_WindowDescC {
        const char* title;
        unsigned width, height;
        int resizable, vsync, visible, highDPI;
    } FrameKit_WindowDescC;

    typedef void* FrameKit_WindowPtr; // opaque IWindow*

    typedef struct FrameKit_WindowPlugin {
        unsigned abi;          // == FRAMEKIT_WINDOW_PLUGIN_ABI
        int id;                // matches FrameKit::WindowBackend numeric
        const char* name;      // "GLFW"/"Win32"/"Cocoa"
        FrameKit_WindowPtr(*create)(const FrameKit_WindowDescC*);
        void (*destroy)(FrameKit_WindowPtr);
    } FrameKit_WindowPlugin;

#if FK_PLATFORM_WINDOWS
    __declspec(dllexport)
#endif
        const FrameKit_WindowPlugin* FrameKit_GetWindowPlugin(void);

#ifdef __cplusplus
}
#endif