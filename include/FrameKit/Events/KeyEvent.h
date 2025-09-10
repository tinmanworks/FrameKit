// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/KeyEvent.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Keyboard input events
// =============================================================================

#pragma once

#include "FrameKit/Events/Event.h"
#include "FrameKit/Input/KeyCodes.h"

#include <string>

namespace FrameKit {

class KeyEvent : public Event {
public:
    KeyCode GetKeyCode() const { return m_Key; }
    int     GetScanCode() const { return m_Scan; }
    int     GetMods()     const { return m_Mods; }
    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
    KeyEvent(KeyCode key, int sc, int mods) : m_Key(key), m_Scan(sc), m_Mods(mods) {}
    KeyCode m_Key; int m_Scan; int m_Mods;
};

class KeyPressedEvent final : public KeyEvent {
public:
    KeyPressedEvent(KeyCode key, int sc, int mods, bool repeat)
      : KeyEvent(key, sc, mods), m_Repeat(repeat) {}
    EVENT_CLASS_TYPE(KeyPressed)
    bool IsRepeat() const { return m_Repeat; }
private: bool m_Repeat;
};

class KeyReleasedEvent final : public KeyEvent {
public:
    KeyReleasedEvent(KeyCode key, int sc, int mods) : KeyEvent(key, sc, mods) {}
    EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent final : public Event {
public:
    explicit KeyTypedEvent(unsigned codepoint) : m_Codepoint(codepoint) {}
    unsigned GetCodepoint() const { return m_Codepoint; }
    std::string ToString() const override { return "KeyTyped: " + std::to_string(m_Codepoint); }
    EVENT_CLASS_TYPE(KeyTyped)
    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
private:
    unsigned m_Codepoint;
};

} // namespace FrameKit