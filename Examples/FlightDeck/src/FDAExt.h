#pragma once

#include <stdint.h>

#include "FrameKit/Engine/Defines.h"

#define SB_IFACE_HOST_V1 "Sandbox.HostV1"
struct SB_HostV1 {
    uint32_t version, size;
    void (*SetImGuiContext)(void* ctx) noexcept; // void* to avoid C++ ABI
    const char* (*GetActiveDocName)() noexcept;
};

#define SB_IFACE_IMGUI_V1 "Sandbox.ImGuiV1"
struct SB_ImGuiV1 {
    uint32_t version, size;
    void (FK_CDECL* SetContext)(void* imgui_ctx) noexcept;
    void (FK_CDECL* DrawUI)() noexcept;
};