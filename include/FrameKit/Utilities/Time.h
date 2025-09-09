// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Utilities/Time.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Duration type, stopwatch, and engine clock.
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"

#include <chrono>

namespace FrameKit {

    // Duration wrapper kept as float seconds for API stability.
    class Timestep {
    public:
        using SecondsF = std::chrono::duration<float>;

        constexpr Timestep() noexcept = default;
        explicit constexpr Timestep(float seconds) noexcept : m_Time(seconds) {}
        explicit constexpr Timestep(SecondsF seconds) noexcept : m_Time(seconds.count()) {}

        static constexpr Timestep FromMilliseconds(float ms) noexcept { return Timestep(ms * 0.001f); }

        // Queries
        FK_NODISCARD constexpr float Seconds() const noexcept { return m_Time; }
        FK_NODISCARD constexpr float Milliseconds() const noexcept { return m_Time * 1000.0f; }

        // Backward-compat
        constexpr operator float() const noexcept { return m_Time; }

        // Arithmetic
        constexpr Timestep& operator+=(Timestep rhs) noexcept { m_Time += rhs.m_Time; return *this; }
        constexpr Timestep& operator-=(Timestep rhs) noexcept { m_Time -= rhs.m_Time; return *this; }
        friend constexpr Timestep operator+(Timestep a, Timestep b) noexcept { return Timestep(a.m_Time + b.m_Time); }
        friend constexpr Timestep operator-(Timestep a, Timestep b) noexcept { return Timestep(a.m_Time - b.m_Time); }
        friend constexpr Timestep operator*(Timestep a, float s) noexcept { return Timestep(a.m_Time * s); }
        friend constexpr Timestep operator*(float s, Timestep a) noexcept { return Timestep(a.m_Time * s); }
        friend constexpr Timestep operator/(Timestep a, float s) noexcept { return Timestep(a.m_Time / s); }

    private:
        float m_Time = 0.0f; // seconds
    };

    // Stopwatch: elapsed since construction or last reset.
    class Timer {
    public:
        using clock = std::chrono::steady_clock;
        using time_point = std::chrono::time_point<clock>;

        Timer() noexcept;

        void Reset() noexcept;
        FK_NODISCARD Timestep Elapsed() const noexcept;
        FK_NODISCARD Timestep Restart() noexcept;

    private:
        time_point m_start;
    };

    // Engine clock: call Tick() once per frame.
    class Clock {
    public:
        using clock = std::chrono::steady_clock;
        using time_point = std::chrono::time_point<clock>;

        Clock() noexcept;

        void Tick() noexcept;          // updates Delta() and Elapsed()
        void Pause(bool p) noexcept;   // freeze delta while paused

        FK_NODISCARD bool     Paused()  const noexcept;
        FK_NODISCARD Timestep Delta()   const noexcept;  // last Tick delta
        FK_NODISCARD Timestep Elapsed() const noexcept;  // since creation

    private:
        time_point m_start;
        time_point m_prev;
        Timestep   m_delta{};
        Timestep   m_elapsed{};
        bool       m_paused{ false };
    };

    // Sleep helper.
    void Sleep(Timestep dt) noexcept;

} // namespace FrameKit
