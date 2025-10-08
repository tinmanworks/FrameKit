// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/FKABI.h
// Author       : George Gil
// Created      : 2025-10-01
// Updated      : 2025-10-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Core C-ABI for addon discovery and host callback with context.
// =============================================================================

#pragma once
#include <stdint.h>
#include "FrameKit/Engine/Defines.h"

typedef void* (FK_CDECL* FK_GetInterfaceCtxFn)(void* ctx, const char* id, uint32_t ver) noexcept;

struct FK_AddonInfo {
    uint32_t    abi_major, abi_minor, abi_patch;
    const char* name;        // UTF-8
};

// Mandatory addon exports
FK_API void  FK_CDECL GetAddonInfo(FK_AddonInfo* out) noexcept;
FK_API void  FK_CDECL SetHostGetterEx(FK_GetInterfaceCtxFn fn, void* ctx) noexcept;  // context-bearing setter
FK_API void* FK_CDECL GetInterface(const char* iface_id, uint32_t min_ver) noexcept; // host pulls addon tables
FK_API void  FK_CDECL ShutdownAddon() noexcept;