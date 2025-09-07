// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/MouseEvent.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Mouse input events: move, scroll, button press/release.
// =============================================================================

#pragma once

#include "FrameKit/Events/Event.h"
#include "FrameKit/Interface/MouseCodes.h"

namespace FrameKit {

    // -- MouseMovedEvent --
    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y) noexcept;
        FK_NODISCARD float GetX() const noexcept;
        FK_NODISCARD float GetY() const noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
    private:
        float m_MouseX{};
        float m_MouseY{};
    };

    // -- MouseScrolledEvent --
    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(float xOffset, float yOffset) noexcept;
        FK_NODISCARD float GetXOffset() const noexcept;
        FK_NODISCARD float GetYOffset() const noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
    private:
        float m_XOffset{};
        float m_YOffset{};
    };

    // -- MouseButtonEvent --
    class MouseButtonEvent : public Event {
    public:
        FK_NODISCARD MouseCode GetMouseButton() const noexcept;
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
    protected:
        explicit MouseButtonEvent(MouseCode button) noexcept;
        MouseCode m_Button{};
    };

    // -- MouseButtonPressedEvent --
    class MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        explicit MouseButtonPressedEvent(MouseCode button) noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    // -- MouseButtonReleasedEvent --
    class MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        explicit MouseButtonReleasedEvent(MouseCode button) noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(MouseButtonReleased)
    };

} // namespace FrameKit
