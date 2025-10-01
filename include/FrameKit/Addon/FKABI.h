// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/FKABI.h
// Author       : George Gil
// Created      : 2025-10-01
// Updated      : 2025-10-01
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        
// =============================================================================
#pragma once
#include <stdint.h>
#include "FrameKit/Engine/Defines.h"

typedef void* (FK_CDECL* FK_GetInterfaceFn)(const char* iface_id, uint32_t min_ver) noexcept;

struct FK_AddonInfo {
    uint32_t abi_major, abi_minor, abi_patch;
    const char* name;        // UTF-8
    const char* build_id;    // git hash or timestamp
};

// Mandatory addon exports
FK_API void  FK_CDECL GetAddonInfo(FK_AddonInfo* out) noexcept;
FK_API void  FK_CDECL SetHostGetter(FK_GetInterfaceFn host_get) noexcept; // host → addon
FK_API void* FK_CDECL GetInterface(const char* iface_id, uint32_t min_ver) noexcept; // host pulls tables
FK_API void  FK_CDECL ShutdownAddon() noexcept;
