// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Utilities/Utility.h
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      General utility macros and helpers.
// =============================================================================

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

// -----------
// Size helpers
// -----------
constexpr std::size_t kB(std::size_t x) noexcept { return x * 1024ull; }
constexpr std::size_t MB(std::size_t x) noexcept { return x * 1024ull * 1024ull; }
constexpr std::size_t GB(std::size_t x) noexcept { return x * 1024ull * 1024ull * 1024ull; }

// -------------
// Bit utilities
// -------------
// 64-bit safe.
constexpr unsigned long long BITU(std::size_t x) noexcept { return 1ull << x; }

// ---------------------------------------
// Enum flag operators (opt-in via macro)
// ---------------------------------------
#define FK_ENABLE_ENUM_FLAGS(EnumType) \
inline constexpr EnumType operator|(EnumType a, EnumType b) noexcept { \
    using U = std::underlying_type_t<EnumType>; \
    return static_cast<EnumType>(static_cast<U>(a) | static_cast<U>(b)); } \
inline constexpr EnumType& operator|=(EnumType& a, EnumType b) noexcept { \
    a = a | b; return a; } \
inline constexpr bool operator&(EnumType a, EnumType b) noexcept { \
    using U = std::underlying_type_t<EnumType>; \
    return (static_cast<U>(a) & static_cast<U>(b)) != 0; }

// ---------------------------------------
// Perfect-forwarding event bind helper
// ---------------------------------------
#define FK_BIND_EVENT_FN(fn) [this](auto&&... args) noexcept(noexcept(this->fn(std::forward<decltype(args)>(args)...))) \
-> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }