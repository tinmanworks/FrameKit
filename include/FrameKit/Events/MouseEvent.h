// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/MouseEvent.h
// Author       : George Gil
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Mouse input events
// =============================================================================

#pragma once

#include "FrameKit/Events/Event.h"
#include "FrameKit/Input/MouseCodes.h"
#include <string>

namespace FrameKit {

class MouseMovedEvent final : public Event {
public:
    MouseMovedEvent(float x, float y) : m_X(x), m_Y(y) {}
    float GetX() const { return m_X; }
    float GetY() const { return m_Y; }
    std::string ToString() const override {
        return "MouseMoved: " + std::to_string(m_X) + "," + std::to_string(m_Y);
    }
    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
    float m_X, m_Y;
};

class MouseScrolledEvent final : public Event {
public:
    MouseScrolledEvent(float dx, float dy) : m_Dx(dx), m_Dy(dy) {}
    float GetXOffset() const { return m_Dx; }
    float GetYOffset() const { return m_Dy; }
    std::string ToString() const override {
        return "MouseScrolled: " + std::to_string(m_Dx) + "," + std::to_string(m_Dy);
    }
    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
    float m_Dx, m_Dy;
};

class MouseButtonEvent : public Event {
public:
    MouseCode GetButton() const { return m_Button; }
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
protected:
    explicit MouseButtonEvent(MouseCode b) : m_Button(b) {}
    MouseCode m_Button;
};

class MouseButtonPressedEvent final : public MouseButtonEvent {
public:
    explicit MouseButtonPressedEvent(MouseCode b) : MouseButtonEvent(b) {}
    std::string ToString() const override { return "MouseButtonPressed: " + std::to_string((int)GetButton()); }
    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent final : public MouseButtonEvent {
public:
    explicit MouseButtonReleasedEvent(MouseCode b) : MouseButtonEvent(b) {}
    std::string ToString() const override { return "MouseButtonReleased: " + std::to_string((int)GetButton()); }
    EVENT_CLASS_TYPE(MouseButtonReleased)
};

} // namespace FrameKit
