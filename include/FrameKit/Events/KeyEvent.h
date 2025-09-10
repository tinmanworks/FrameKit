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

#include <string>

namespace FrameKit {

class KeyEvent : public Event {
public:
    int GetKeyCode()   const { return m_KeyCode; }   // virtual-key or GLFW key
    int GetScanCode()  const { return m_ScanCode; }  // hardware scancode when available
    int GetMods()      const { return m_Mods; }      // bitmask from backend (e.g. shift/ctrl/alt/super)
    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
    KeyEvent(int key, int sc, int mods) : m_KeyCode(key), m_ScanCode(sc), m_Mods(mods) {}
    int m_KeyCode, m_ScanCode, m_Mods;
};

class KeyPressedEvent final : public KeyEvent {
public:
    KeyPressedEvent(int key, int sc, int mods, bool repeat)
        : KeyEvent(key, sc, mods), m_Repeat(repeat) {}
    bool IsRepeat() const { return m_Repeat; }
    std::string ToString() const override {
        return "KeyPressed: key=" + std::to_string(GetKeyCode()) +
               " sc=" + std::to_string(GetScanCode()) +
               " mods=" + std::to_string(GetMods()) +
               (m_Repeat ? " (repeat)" : "");
    }
    EVENT_CLASS_TYPE(KeyPressed)
private:
    bool m_Repeat;
};

class KeyReleasedEvent final : public KeyEvent {
public:
    KeyReleasedEvent(int key, int sc, int mods) : KeyEvent(key, sc, mods) {}
    std::string ToString() const override {
        return "KeyReleased: key=" + std::to_string(GetKeyCode()) +
               " sc=" + std::to_string(GetScanCode()) +
               " mods=" + std::to_string(GetMods());
    }
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