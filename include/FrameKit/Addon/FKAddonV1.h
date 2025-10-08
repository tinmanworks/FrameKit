// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/FKAddonV1.h
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Standard lifecycle interface table for addons (V1).
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"
#include <stdint.h>

#define FK_IFACE_ADDON_V1 "FrameKit.Addon.V1"
struct FK_AddonV1 {
    uint32_t version;
    uint32_t size;
    void (FK_CDECL* Initialize)(void) noexcept;
    void (FK_CDECL* OnUpdate)(void) noexcept;
    void (FK_CDECL* OnRender)(void) noexcept;
    void (FK_CDECL* OnCyclic)(void) noexcept;
    void (FK_CDECL* Shutdown)(void) noexcept;
};
