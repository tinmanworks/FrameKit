// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Events/KeyEvent.cpp
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Keyboard events: press/release/typed, with minimal, modern API.
// =============================================================================

#pragma once

#include "FrameKit/Events/KeyEvent.h"

#include <sstream>
#include <string>
#include <type_traits>

namespace FrameKit {

    // -- KeyEvent --
    KeyCode KeyEvent::GetKeyCode() const noexcept { return m_KeyCode; }

    KeyEvent::KeyEvent(KeyCode keycode) noexcept : m_KeyCode(keycode) {}

    // -- KeyPressedEvent --
    KeyPressedEvent::KeyPressedEvent(KeyCode keycode, bool isRepeat) noexcept
        : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

    bool KeyPressedEvent::IsRepeat() const noexcept { return m_IsRepeat; }

    std::string KeyPressedEvent::ToString() const {
        std::stringstream ss;
        using U = std::underlying_type_t<KeyCode>;
        ss << "KeyPressedEvent: " << static_cast<U>(m_KeyCode)
           << " (repeat = " << (m_IsRepeat ? "true" : "false") << ")";
        return ss.str();
    }

    // -- KeyReleasedEvent --
    KeyReleasedEvent::KeyReleasedEvent(KeyCode keycode) noexcept
        : KeyEvent(keycode) {}

    std::string KeyReleasedEvent::ToString() const {
        std::stringstream ss;
        using U = std::underlying_type_t<KeyCode>;
        ss << "KeyReleasedEvent: " << static_cast<U>(m_KeyCode);
        return ss.str(); 
    }

    // -- KeyTypedEvent --
    KeyTypedEvent::KeyTypedEvent(KeyCode keycode) noexcept
        : KeyEvent(keycode) {}

    std::string KeyTypedEvent::ToString() const {
        std::stringstream ss;
        using U = std::underlying_type_t<KeyCode>;
        ss << "KeyTypedEvent: " << static_cast<U>(m_KeyCode);
        return ss.str();
    }

} // namespace FrameKit
