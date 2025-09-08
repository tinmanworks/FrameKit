// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Utilities/Time.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Duration type, stopwatch, and engine clock.
// =============================================================================

#include "FrameKit/Utilities/Time.h"

#include <thread>

namespace FrameKit {

    // -------- Timer --------
    Timer::Timer() noexcept
        : m_start(clock::now()) {
    }

    void Timer::Reset() noexcept {
        m_start = clock::now();
    }

    Timestep Timer::Elapsed() const noexcept {
        return Timestep(std::chrono::duration<float>(clock::now() - m_start));
    }

    Timestep Timer::Restart() noexcept {
        auto now = clock::now();
        auto dt = now - m_start;
        m_start = now;
        return Timestep(std::chrono::duration<float>(dt));
    }

    // -------- Clock --------
    Clock::Clock() noexcept
        : m_start(clock::now()), m_prev(m_start) {
    }

    void Clock::Tick() noexcept {
        auto now = clock::now();

        if (m_paused) {
            m_delta = Timestep{};
            m_prev = now;            // prevent spike on resume
            return;
        }

        m_delta = Timestep(std::chrono::duration<float>(now - m_prev));
        m_elapsed = Timestep(std::chrono::duration<float>(now - m_start));
        m_prev = now;
    }

    void Clock::Pause(bool p) noexcept {
        if (p && !m_paused) {
            m_paused = true;
        }
        else if (!p && m_paused) {
            m_paused = false;
            m_prev = clock::now();    // resume cleanly
        }
    }

    bool Clock::Paused() const noexcept { return m_paused; }
    Timestep Clock::Delta() const noexcept { return m_delta; }
    Timestep Clock::Elapsed() const noexcept { return m_elapsed; }

    // -------- Sleep --------
    void Sleep(Timestep dt) noexcept {
        using SecondsF = std::chrono::duration<float>;
        if (dt.Seconds() <= 0.0f) return;
        std::this_thread::sleep_for(SecondsF{ dt.Seconds() });
    }

} // namespace FrameKit
