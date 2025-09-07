// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/Event.h
// Author       : George Gil
// Created      : 2025-08-11
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Public event API (types, base class, dispatcher interface).
// =============================================================================

#pragma once

#include "FrameKit/Base/Base.h"

#include <string>

namespace FrameKit
{
    // Events in FrameKit are currently blocking, meaning when an event occurs
    // it immediately gets dispatched and must be dealt with right then and there.
    // For the future, a better strategy might be to buffer events in an event
    // bus and process them during the "event" part of the update stage.

    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        UpdateState, UpdateParameter
    };

    enum EventCategory
    {
        None = 0,
        EventCategoryApplication    = FK_BIT(0),
        EventCategoryInput          = FK_BIT(1),
        EventCategoryKeyboard       = FK_BIT(2),
        EventCategoryMouse          = FK_BIT(3),
        EventCategoryMouseButton    = FK_BIT(4),
        EventCategoryInterprocess   = FK_BIT(5)
    };

#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; } \
    EventType GetEventType() const override { return GetStaticType(); } \
    const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    int GetCategoryFlags() const override { return category; }

    class Event
    {
    public:
        virtual ~Event() = default;

        bool Handled = false;

        FK_NODISCARD virtual EventType   GetEventType() const = 0;
        FK_NODISCARD virtual const char* GetName() const = 0;
        FK_NODISCARD virtual int         GetCategoryFlags() const = 0;

        FK_NODISCARD virtual std::string ToString() const;

        FK_NODISCARD bool IsInCategory(EventCategory category) const;
    };

    class EventDispatcher
    {
    public:
        explicit EventDispatcher(Event& event);

        template<typename T, typename F>
        bool Dispatch(const F& func) {
            T* castedEvent = dynamic_cast<T*>(&m_Event);
            if (castedEvent) {
                m_Event.Handled |= func(*castedEvent);
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

    std::ostream& operator<<(std::ostream& os, const Event& e);


} // namespace FrameKit
