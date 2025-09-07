// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/KeyEvent.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Keyboard events: press/release/typed, with minimal, modern API.
// =============================================================================

#pragma once

#include "FrameKit/Events/Event.h"
#include "FrameKit/Interface/KeyCodes.h"

namespace FrameKit {

    // -- KeyEvent --
    class KeyEvent : public Event {
    public:
        FK_NODISCARD KeyCode GetKeyCode() const noexcept;
        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
    protected:
        explicit KeyEvent(KeyCode keycode) noexcept;
        KeyCode m_KeyCode{};
    };

    // -- KeyPressedEvent --
    class KeyPressedEvent : public KeyEvent {
    public:
        explicit KeyPressedEvent(KeyCode keycode, bool isRepeat = false) noexcept;
        FK_NODISCARD bool IsRepeat() const noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(KeyPressed)
    private:
        bool m_IsRepeat{ false };
    };

    // -- KeyReleasedEvent --
    class KeyReleasedEvent : public KeyEvent {
    public:
        explicit KeyReleasedEvent(KeyCode keycode) noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(KeyReleased)
    };

    // -- KeyTypedEvent --
    class KeyTypedEvent : public KeyEvent {
    public:
        explicit KeyTypedEvent(KeyCode keycode) noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(KeyTyped)
    };

} // namespace FrameKit
