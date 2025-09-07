// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Utils/Timestep.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Lightweight time interval wrapper used across the engine.
//      Keeps the original float-based API, and adds std::chrono support.
// =============================================================================

#pragma once

#include <chrono>

namespace FrameKit
{
    class Timestep
    {
    public:
        using SecondsF = std::chrono::duration<float>;

        // Constructors
        constexpr Timestep() noexcept = default;
        explicit constexpr Timestep(float seconds) noexcept
            : m_Time(seconds) {
        }
        explicit constexpr Timestep(SecondsF seconds) noexcept
            : m_Time(seconds.count()) {
        }

        // Factory helpers
        static constexpr Timestep FromMilliseconds(float ms) noexcept {
            return Timestep(ms / 1000.0f);
        }

        // Implicit conversion preserved for backward compatibility
        constexpr operator float() const noexcept { return m_Time; }

        // Preferred modern accessors
        [[nodiscard]] constexpr float Seconds() const noexcept { return m_Time; }
        [[nodiscard]] constexpr float Milliseconds() const noexcept { return m_Time * 1000.0f; }

        // Backward-compatible names (kept for existing code)
        [[nodiscard]] constexpr float GetSeconds() const noexcept { return Seconds(); }
        [[nodiscard]] constexpr float GetMilliseconds() const noexcept { return Milliseconds(); }

    private:
        float m_Time = 0.0f; // seconds
    };
}
