// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Engine/Utility.h 
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-09
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      General utility macros and helpers.
// =============================================================================

#pragma once
#include <cstddef>
#include <utility>

// -------------
// Bit utilities
// -------------
#define BIT(x) (1u << (x))

constexpr std::size_t kB(std::size_t x) noexcept { return x * 1024ull; }
constexpr std::size_t MB(std::size_t x) noexcept { return x * 1024ull * 1024ull; }
constexpr std::size_t GB(std::size_t x) noexcept { return x * 1024ull * 1024ull * 1024ull; }

// ---------------------------------------
// Perfect-forwarding event bind helper
// ---------------------------------------
#define FK_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { \
    return this->fn(std::forward<decltype(args)>(args)...); \
}
