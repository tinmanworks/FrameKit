// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/ApplicationEvent.h
// Author       : George Gil
// Created      : 2025-08-11
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Core application-level events (window lifecycle & app ticks).
// =============================================================================

#pragma once

#include "FrameKit/Events/Event.h"

#include <cstdint>
#include <sstream>
#include <string>

namespace FrameKit {

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(std::uint32_t width, std::uint32_t height) noexcept
            : m_Width(width), m_Height(height) {
        }

        std::uint32_t GetWidth()  const noexcept { return m_Width; }
        std::uint32_t GetHeight() const noexcept { return m_Height; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
            EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        std::uint32_t m_Width{};
        std::uint32_t m_Height{};
    };

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
            EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppTickEvent : public Event {
    public:
        AppTickEvent() = default;

        EVENT_CLASS_TYPE(AppTick)
            EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppUpdateEvent : public Event {
    public:
        AppUpdateEvent() = default;

        EVENT_CLASS_TYPE(AppUpdate)
            EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppRenderEvent : public Event {
    public:
        AppRenderEvent() = default;

        EVENT_CLASS_TYPE(AppRender)
            EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

} // namespace FrameKit
