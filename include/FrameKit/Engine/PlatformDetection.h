// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Engine/PlatformDetection.h 
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Platform and architecture detection macros for FrameKit.
// =============================================================================

#pragma once

// ---------------------
// Platform detection
// ---------------------

#if defined(_WIN32) || defined(_WIN64)
#define FK_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#define FK_PLATFORM_MACOS 1
#elif defined(__linux__)
#define FK_PLATFORM_LINUX 1
#else
#error "Unsupported platform"
#endif


// ---------------------
// Architecture detection
// ---------------------
#if defined(_M_X64) || defined(__x86_64__) || defined(__aarch64__) || defined(_M_ARM64)
#define FK_ARCH_64 1
#else
#define FK_ARCH_32 1
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#define FK_ARCH_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)
#define FK_ARCH_ARM 1
#else
#error "Unsupported architecture"
#endif