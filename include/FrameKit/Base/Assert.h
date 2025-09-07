// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Base/Assert.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//         Assertion macros for runtime checks with logging and debug break.
// =============================================================================

#pragma once

#include "FrameKit/Debug/Log.h"
#include <filesystem>

#ifdef FK_ENABLE_ASSERTS

// -----------------------------------------------------------------------------
// Internal implementation: pass the logger macro (FK_ERROR or FK_CORE_ERROR)
// -----------------------------------------------------------------------------
#define FK_INTERNAL_ASSERT_IMPL(LOG_FN, CHECK, ...)                                     \
    do {                                                                                \
        if (!(static_cast<bool>(CHECK))) {                                              \
            LOG_FN(__VA_ARGS__);                                                        \
            FK_DEBUGBREAK();                                                            \
        }                                                                               \
    } while (0)

// With custom message: prefixes with "Assertion failed: "
#define FK_INTERNAL_ASSERT_WITH_MSG(LOG_FN, CHECK, ...)                                 \
    FK_INTERNAL_ASSERT_IMPL(LOG_FN, CHECK, "Assertion failed: {}", __VA_ARGS__)

// Without custom message: prints expression and file:line
#define FK_INTERNAL_ASSERT_NO_MSG(LOG_FN, CHECK)                                        \
    FK_INTERNAL_ASSERT_IMPL(                                                            \
        LOG_FN, CHECK,                                                                  \
        "Assertion '{}' failed at {}:{}",                                               \
        FK_STRINGIFY_MACRO(CHECK),                                                      \
        std::filesystem::path(__FILE__).filename().string(),                            \
        __LINE__                                                                        \
    )

// -----------------------------------------------------------------------------
// Macro selector trick:
// If user provided (CHECK, msg, ...), pick WITH_MSG; otherwise pick NO_MSG.
// Works on both MSVC and GCC/Clang without needing __VA_OPT__ or comma-swallow.
// -----------------------------------------------------------------------------
#define FK_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define FK_INTERNAL_ASSERT_GET_MACRO(...)                                                \
    FK_EXPAND_MACRO(FK_INTERNAL_ASSERT_GET_MACRO_NAME(                                   \
        __VA_ARGS__,                                                                     \
        FK_INTERNAL_ASSERT_WITH_MSG,                                                     \
        FK_INTERNAL_ASSERT_NO_MSG))

// -----------------------------------------------------------------------------
// Public asserts
// Usage examples:
//   FK_ASSERT(x > 0);
//   FK_ASSERT(x > 0, "x was {}", x);
//   FK_CORE_ASSERT(ptr != nullptr);
//   FK_CORE_ASSERT(ok, "init failed: code {}", rc);
// -----------------------------------------------------------------------------
#define FK_ASSERT(...)                                                                   \
    FK_EXPAND_MACRO(FK_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(FK_ERROR, __VA_ARGS__))

#define FK_CORE_ASSERT(...)                                                              \
    FK_EXPAND_MACRO(FK_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(FK_CORE_ERROR, __VA_ARGS__))

#else
#define FK_ASSERT(...)
#define FK_CORE_ASSERT(...)
#endif
