// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Engine/Assert.h 
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-09
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Assertion macros built on FK_DEBUG and FK_DEBUGTRAP.
// =============================================================================

#pragma once
#include "FrameKit/Engine/Defines.h"

#ifndef FK_ENABLE_ASSERTS
#define FK_ENABLE_ASSERTS FK_DEBUG
#endif

#if FK_ENABLE_ASSERTS
#define FK_ASSERT(cond)           \
        do {                          \
            if (!(cond)) {            \
                FK_DEBUGTRAP();       \
            }                         \
        } while (0)
#else
#define FK_ASSERT(cond) ((void)0)
#endif

#ifndef FK_VERIFY
#if FK_DEBUG
#define FK_VERIFY(cond) FK_ASSERT(cond)
#else
#define FK_VERIFY(cond) ((void)(cond))
#endif
#endif

#ifndef FK_UNREACHABLE
#if defined(__has_builtin) && __has_builtin(__builtin_unreachable)
#define FK_UNREACHABLE() do { FK_DEBUGTRAP(); __builtin_unreachable(); } while (0)
#elif defined(_MSC_VER)
#define FK_UNREACHABLE() do { FK_DEBUGTRAP(); __assume(0); } while (0)
#else
#define FK_UNREACHABLE() do { FK_DEBUGTRAP(); } while (0)
#endif
#endif
