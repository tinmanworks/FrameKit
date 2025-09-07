// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/ApplicationEvent.h
// Author       : George Gil
// Created      : 2025-08-11
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Core application-level events (window lifecycle & app ticks).
// =============================================================================

#pragma once

#include "FrameKit/Events/Event.h"

namespace FrameKit {

    // -- WindowResizeEvent --
    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(std::uint32_t width, std::uint32_t height) noexcept;
        std::uint32_t GetWidth()  const noexcept;
        std::uint32_t GetHeight() const noexcept;
        std::string ToString() const override;
        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        std::uint32_t m_Width{};
        std::uint32_t m_Height{};
    };

    // -- WindowCloseEvent --
    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;
        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    // -- AppTickEvent --
    class AppTickEvent : public Event {
    public:
        AppTickEvent() = default;
        EVENT_CLASS_TYPE(AppTick)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    // -- AppUpdateEvent --
    class AppUpdateEvent : public Event {
    public:
        AppUpdateEvent() = default;
        EVENT_CLASS_TYPE(AppUpdate)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    // -- AppRenderEvent --
    class AppRenderEvent : public Event {
    public:
        AppRenderEvent() = default;
        EVENT_CLASS_TYPE(AppRender)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

} // namespace FrameKit
