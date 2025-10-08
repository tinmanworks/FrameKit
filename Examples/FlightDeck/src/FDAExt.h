// =============================================================================
// Project      : FlightDeck
// File         : FDAExt.h
// Author       : George Gil
// Created      : 2025-10-08
// Updated      : 2025-10-08
// License      : Application code using FrameKit
// Description  : Sandbox host + addon-specific interfaces.
// =============================================================================
#pragma once
#include <stdint.h>

// ---- ImGui host table the addon can query ----
#define SB_IFACE_IMGUI_HOST_V1 "Sandbox.ImGuiHost.V1"
struct SB_ImGuiHostV1 {
    uint32_t version, size;
    void* (*GetImGuiContext)() noexcept;             // returns ImGuiContext*
};