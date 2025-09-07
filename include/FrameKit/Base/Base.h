// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Base/Base.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Core type defs, attributes, and helpers for FrameKit.
// =============================================================================

#pragma once

#include "FrameKit/Platform/PlatformDetection.h"

#include <cstddef>   // std::size_t
#include <cstdint>
#include <memory>
#include <utility>

// -----------------------------------
// Compiler / attributes
// -----------------------------------
#if defined(_MSC_VER)
  #define FK_FORCE_INLINE __forceinline
  #define FK_DEBUGTRAP() __debugbreak()
  #define FK_ASSUME(x) __assume(x)
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
  #define FK_DEBUGTRAP() ((void)0)
  #define FK_ASSUME(x)   ((void)0)
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

#define FK_EXPAND_MACRO(x)   x
#define FK_STRINGIFY_MACRO(x) #x

// -----------------------
// Debug break & asserts
// -----------------------
#if FK_DEBUG
  #define FK_DEBUGBREAK() FK_DEBUGTRAP()
  #define FK_ENABLE_ASSERTS
#else
  #define FK_DEBUGBREAK() ((void)0)
#endif

// ---------------------------
// Smart pointer conveniences
// ---------------------------
namespace FrameKit {

    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename... Args>
    FK_FORCE_INLINE Scope<T> CreateScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename... Args>
    FK_FORCE_INLINE Ref<T> CreateRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    // ------------- 
    // Bit utility
    // -------------
    constexpr unsigned FK_BIT(unsigned x) noexcept { return 1u << x; }

    // -----------------
    // Size conversions
    // -----------------
    constexpr std::size_t KB(std::size_t x) noexcept { return x * static_cast<std::size_t>(1024); }
    constexpr std::size_t MB(std::size_t x) noexcept { return KB(x) * static_cast<std::size_t>(1024); }
    constexpr std::size_t GB(std::size_t x) noexcept { return MB(x) * static_cast<std::size_t>(1024); }

    // ---------------------------------------
    // Perfect-forwarding event bind helper
    // ---------------------------------------
    #define FK_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { \
        return this->fn(std::forward<decltype(args)>(args)...);             \
    }

} // namespace FrameKit
