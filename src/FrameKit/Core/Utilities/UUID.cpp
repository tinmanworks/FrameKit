// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Utilities/UUID.cpp
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 64-bit UUID wrapper with random generation, comparisons,
//                and std::hash support for unordered containers.
// =============================================================================

#include "FrameKit/Utilities/UUID.h"

#include <random>

namespace FrameKit
{
    // Internal PRNG state for UUID generation.
    // std::random_device may be slow or deterministic on some platforms,
    // but this is portable across Windows/Linux without extra deps.
    static std::random_device rd;
    static std::mt19937_64 engine(rd());
    static std::uniform_int_distribution<std::uint64_t> dist;

    UUID::UUID()
        : m_UUID(dist(engine))
    {
    }
}
