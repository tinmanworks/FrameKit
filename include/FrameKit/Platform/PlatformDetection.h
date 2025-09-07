// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Platform/PlatformDetection.h
// Author       : George Gil
// Created      : 2025-08-09
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//      Unified platform/arch/compiler detection for FrameKit.
//      Primary source is CMake-provided macros; falls back to
//      compiler built-ins if CMake macros are absent.
// =============================================================================
//  Provided macros:
//    FK_PLATFORM_WINDOWS | FK_PLATFORM_LINUX
//    FK_ARCH_64 | FK_ARCH_32
//    FK_COMPILER_MSVC | FK_COMPILER_GNU | FK_COMPILER_CLANG
//    FK_DEBUG (defined when not NDEBUG)
//    FK_OS_STRING, FK_ARCH_STRING, FK_COMPILER_STRING (string literals)
// =============================================================================

#pragma once

// ---------- Platform (prefer CMake, then compiler built-ins) ----------
#ifndef FK_PLATFORM_WINDOWS
#ifndef FK_PLATFORM_LINUX
  #if defined(CMAKE_PLATFORM_WIN)
    #define FK_PLATFORM_WINDOWS 1
  #elif defined(CMAKE_PLATFORM_LINUX)
    #define FK_PLATFORM_LINUX 1
  #else
    #if defined(_WIN32) || defined(_WIN64)
      #define FK_PLATFORM_WINDOWS 1
    #elif defined(__linux__)
      #define FK_PLATFORM_LINUX 1
    #endif
  #endif
#endif
#endif

// ---------- Architecture (prefer CMake, then built-ins) ----------
#ifndef FK_ARCH_64
#ifndef FK_ARCH_32
  #if defined(CMAKE_PLATFORM_64_BIT)
    #define FK_ARCH_64 1
  #elif defined(CMAKE_PLATFORM_32_BIT)
    #define FK_ARCH_32 1
  #else
    #if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || \
        defined(__ppc64__) || defined(__LP64__)
      #define FK_ARCH_64 1
    #else
      #define FK_ARCH_32 1
    #endif
  #endif
#endif
#endif

// ------------------------------ Compiler --------------------------------
#ifndef FK_COMPILER_STRING
  #if defined(_MSC_VER)
    #define FK_COMPILER_MSVC 1
    #define FK_COMPILER_STRING "MSVC"
    #define FK_COMPILER_VERSION _MSC_VER
  #elif defined(__clang__)
    #define FK_COMPILER_CLANG 1
    #define FK_COMPILER_STRING "Clang"
    #define FK_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
  #elif defined(__GNUC__)
    #define FK_COMPILER_GNU 1
    #define FK_COMPILER_STRING "GCC"
    #define FK_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
  #else
    #define FK_COMPILER_STRING "UnknownCompiler"
    #define FK_COMPILER_VERSION 0
  #endif
#endif

// ------------------------------ Build mode ------------------------------
#ifndef FK_DEBUG
  #if !defined(NDEBUG)
    #define FK_DEBUG 1
  #else
    #define FK_DEBUG 0
  #endif
#endif

// ------------------------------ Strings ---------------------------------
#ifndef FK_OS_STRING
  #if defined(FK_PLATFORM_WINDOWS)
    #define FK_OS_STRING "Windows"
  #elif defined(FK_PLATFORM_LINUX)
    #define FK_OS_STRING "Linux"
  #else
    #define FK_OS_STRING "UnknownOS"
  #endif
#endif

#ifndef FK_ARCH_STRING
  #if defined(FK_ARCH_64)
    #define FK_ARCH_STRING "x64"
  #else
    #define FK_ARCH_STRING "x86"
  #endif
#endif

// --------- Hard stop for unsupported OS, opt-out with FK_ALLOW_UNKNOWN_PLATFORM ---------
#if !defined(FK_PLATFORM_WINDOWS) && !defined(FK_PLATFORM_LINUX)
  #if !defined(FK_ALLOW_UNKNOWN_PLATFORM)
    #error "Unsupported platform: only Windows and Linux are currently supported."
  #endif
#endif