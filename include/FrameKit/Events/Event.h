// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/Event.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Event base class
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"
#include "FrameKit/Utilities/Utilities.h"

#include <functional>
#include <string>
#include <ostream>
#include <cstdint>

namespace FrameKit {

    enum class EventType : int {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        UpdateState, UpdateParameter
    };

    using EventCategoryBits = std::uint64_t;

    enum EventCategory : EventCategoryBits {
        None                    = 0,
        EventCategoryApplication= BITU(0),
        EventCategoryInput      = BITU(1),
        EventCategoryKeyboard   = BITU(2),
        EventCategoryMouse      = BITU(3),
        EventCategoryMouseButton= BITU(4),
        EventCategoryInterprocess=BITU(5)
    };

#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; } \
    EventType GetEventType() const override { return GetStaticType(); } \
    const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    EventCategoryBits GetCategoryFlags() const override { return static_cast<EventCategoryBits>(category); }

    class Event {
    public:
        virtual ~Event() = default;
        bool Handled = false;

        FK_NODISCARD virtual EventType GetEventType() const = 0;
        FK_NODISCARD virtual const char* GetName() const = 0;
        FK_NODISCARD virtual EventCategoryBits GetCategoryFlags() const = 0;
        FK_NODISCARD virtual std::string ToString() const { return GetName(); }

        FK_NODISCARD bool IsInCategory(EventCategory category) const {
            return (GetCategoryFlags() & static_cast<EventCategoryBits>(category)) != 0;
        }
    };

    class EventDispatcher {
    public:
        explicit EventDispatcher(Event& e) : m_Event(e) {}
        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_Event.GetEventType() == T::GetStaticType()) {
                m_Event.Handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.ToString();
    }

} // namespace FrameKit