// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Events/ApplicationEvent.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Core application-level event implementations (window lifecycle & app ticks).
// =============================================================================

#pragma once

#include "FrameKit/Events/ApplicationEvent.h"

#include <cstdint>
#include <sstream>
#include <string>

namespace FrameKit {

    // -- WindowResizeEvent --
    WindowResizeEvent::WindowResizeEvent(std::uint32_t width, std::uint32_t height) noexcept
        : m_Width(width), m_Height(height) {}

    std::uint32_t WindowResizeEvent::GetWidth() const noexcept { return m_Width; }

    std::uint32_t WindowResizeEvent::GetHeight() const noexcept { return m_Height; }

    std::string WindowResizeEvent::ToString() const {
        std::ostringstream oss;
        oss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
        return oss.str();
    }

} // namespace FrameKit
