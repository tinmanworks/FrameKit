// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Engine/Defines.h 
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-09
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Core defines: build config toggles, compiler attributes, visibility.
// =============================================================================

#pragma once
#include "FrameKit/Engine/PlatformDetection.h"

// ---------------------
// Configuration toggles
// ---------------------
#if !defined(FK_DEBUG)
#if defined(_DEBUG) || !defined(NDEBUG)
#define FK_DEBUG 1
#else
#define FK_DEBUG 0
#endif
#endif

// ---------------------
// Compiler / attributes
// ---------------------
#if defined(_MSC_VER)
#define FK_FORCE_INLINE __forceinline
#define FK_DEBUGTRAP()  __debugbreak()
#define FK_ASSUME(x)    __assume(x)
#elif defined(__GNUC__) || defined(__clang__)
#define FK_FORCE_INLINE inline __attribute__((always_inline))
#if defined(__has_builtin) && __has_builtin(__builtin_trap)
#define FK_DEBUGTRAP() __builtin_trap()
#else
#define FK_DEBUGTRAP() ((void)0)
#endif
#if defined(__has_builtin) && __has_builtin(__builtin_assume)
#define FK_ASSUME(x) __builtin_assume(x)
#else
#define FK_ASSUME(x) ((void)0)
#endif
#else
#define FK_FORCE_INLINE inline
#define FK_DEBUGTRAP()  ((void)0)
#define FK_ASSUME(x)    ((void)0)
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard)
#define FK_NODISCARD [[nodiscard]]
#else
#define FK_NODISCARD
#endif

#if defined(__GNUC__) || defined(__clang__)
#define FK_LIKELY(x)   __builtin_expect(!!(x), 1)
#define FK_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define FK_LIKELY(x)   (x)
#define FK_UNLIKELY(x) (x)
#endif

#define FK_EXPAND_MACRO(x)    x
#define FK_STRINGIFY_MACRO(x) #x
#define FK_CONCAT_IMPL(a,b)   a##b
#define FK_CONCAT(a,b)        FK_CONCAT_IMPL(a,b)

// ---------------------
// Symbol visibility
// ---------------------
#if FK_PLATFORM_WINDOWS
#if defined(FK_BUILD_DLL)
#define FK_API extern "C" __declspec(dllexport)
#else
#define FK_API extern "C" __declspec(dllimport)
#endif
#define FK_LOCAL
#define FK_CDECL __cdecl
#else
#if defined(__GNUC__) || defined(__clang__)
#define FK_API   extern "C" __attribute__((visibility("default")))
#define FK_LOCAL __attribute__((visibility("hidden")))
#else
#define FK_API
#define FK_LOCAL
#endif
#define FK_CDECL
#endif

// ---------------------
// Misc attributes
// ---------------------
#if defined(__has_cpp_attribute) && __has_cpp_attribute(fallthrough)
#define FK_FALLTHROUGH [[fallthrough]]
#else
#define FK_FALLTHROUGH ((void)0)
#endif

#if defined(_MSC_VER)
#define FK_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__) || defined(__clang__)
#define FK_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define FK_DEPRECATED(msg)
#endif
