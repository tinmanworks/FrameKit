/*
 * Project: FrameKit
 * File: UUID.cpp
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   Random UUID generation implementation using C++ standard facilities.
 */

#include "FrameKit/Core/UUID.h"

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
