// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Events/MouseEvent.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Mouse input events: move, scroll, button press/release.
// =============================================================================

#pragma once

#include "FrameKit/Events/MouseEvent.h"

#include <sstream>
#include <string>
#include <type_traits>

namespace FrameKit {

    // -- MouseMovedEvent --
    MouseMovedEvent::MouseMovedEvent(float x, float y) noexcept : m_MouseX(x), m_MouseY(y) {}

    float MouseMovedEvent::GetX() const noexcept { return m_MouseX; }

    float MouseMovedEvent::GetY() const noexcept { return m_MouseY; }

    std::string MouseMovedEvent::ToString() const {
        std::ostringstream oss;
        oss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
        return oss.str();
    }

    // -- MouseScrolledEvent --
    MouseScrolledEvent::MouseScrolledEvent(float xOffset, float yOffset) noexcept : m_XOffset(xOffset), m_YOffset(yOffset) {}

    float MouseScrolledEvent::GetXOffset() const noexcept { return m_XOffset; }

    float MouseScrolledEvent::GetYOffset() const noexcept { return m_YOffset; }

    std::string MouseScrolledEvent::ToString() const {
        std::ostringstream oss;
        oss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
        return oss.str();
    }

    // -- MouseButtonEvent --
    MouseButtonEvent::MouseButtonEvent(MouseCode button) noexcept : m_Button(button) {}

    MouseCode MouseButtonEvent::GetMouseButton() const noexcept { return m_Button; }

    // -- MouseButtonPressedEvent --
    MouseButtonPressedEvent::MouseButtonPressedEvent(MouseCode button) noexcept : MouseButtonEvent(button) {}

    std::string MouseButtonPressedEvent::ToString() const {
        std::ostringstream oss;
        oss << "MouseButtonPressedEvent: " << static_cast<int>(m_Button);
        return oss.str();
    }

    // -- MouseButtonReleasedEvent --
    MouseButtonReleasedEvent::MouseButtonReleasedEvent(MouseCode button) noexcept : MouseButtonEvent(button) {}

    std::string MouseButtonReleasedEvent::ToString() const {
        std::ostringstream oss;
        oss << "MouseButtonReleasedEvent: " << static_cast<int>(m_Button);
        return oss.str();
    }
    

} // namespace FrameKit
