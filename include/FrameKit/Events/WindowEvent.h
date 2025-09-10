// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/WindowEvent.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Window events (resize, close, focus, etc.)
// =============================================================================

#pragma once
#include "FrameKit/Events/Event.h"
#include <string>

namespace FrameKit {

class WindowResizeEvent final : public Event {
public:
    WindowResizeEvent(uint32_t w, uint32_t h) : m_W(w), m_H(h) {}
    uint32_t GetWidth() const { return m_W; }
    uint32_t GetHeight() const { return m_H; }
    std::string ToString() const override {
        return "WindowResize: " + std::to_string(m_W) + "x" + std::to_string(m_H);
    }
    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
private: uint32_t m_W, m_H;
};

class WindowCloseEvent final : public Event {
public: EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowFocusEvent final : public Event {
public: EVENT_CLASS_TYPE(WindowFocus)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowLostFocusEvent final : public Event {
public: EVENT_CLASS_TYPE(WindowLostFocus)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowMovedEvent final : public Event {
public:
    WindowMovedEvent(int x, int y) : m_X(x), m_Y(y) {}
    int GetX() const { return m_X; }
    int GetY() const { return m_Y; }
    std::string ToString() const override {
        return "WindowMoved: " + std::to_string(m_X) + "," + std::to_string(m_Y);
    }
    EVENT_CLASS_TYPE(WindowMoved)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
private: int m_X, m_Y;
};

} // namespace FrameKit
