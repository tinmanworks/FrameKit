// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Utils/Timer.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Timer utility class for measuring elapsed time. Uses high-resolution clock.
// =============================================================================

#pragma once

#include <chrono>

namespace FrameKit
{
    class Timer
    {
    public:
        using clock = std::chrono::high_resolution_clock;
        using time_point = std::chrono::time_point<clock>;

        Timer() noexcept {
            Reset();
        }

        void Reset() noexcept {
            m_Start = clock::now();
        }

        [[nodiscard]] float Elapsed() const noexcept {
            return std::chrono::duration<float>(clock::now() - m_Start).count();
        }

        [[nodiscard]] float ElapsedMillis() const noexcept {
            return Elapsed() * 1000.0f;
        }

    private:
        time_point m_Start;
    };
}
