// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/FKHostV1.h
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-01
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Host for FrameKit addons.
// =============================================================================

#pragma once

#include <stdint.h>

#define FK_IFACE_HOST_V1 "FrameKit.HostV1"
struct FK_HostV1 {
    uint32_t version;
    uint32_t size;
    void* (*Alloc)(uint64_t) noexcept;
    void   (*Free)(void*) noexcept;
    void   (*Log)(int level, const char* msg) noexcept;
    double (*NowSeconds)() noexcept;
};
