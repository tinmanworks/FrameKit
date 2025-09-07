// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Events/Event.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Private definitions for the public Event API.
// =============================================================================

#include "FrameKit/Events/Event.h"

#include <ostream>
#include <string>

namespace FrameKit
{
    // Event
    std::string Event::ToString() const {
        return GetName();
    }

    bool Event::IsInCategory(EventCategory category) const {
        return (GetCategoryFlags() & category) != 0;
    }

    // EventDispatcher
    EventDispatcher::EventDispatcher(Event& event)
        : m_Event(event) {}

    // ostream
    std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.ToString();
    }

} // namespace FrameKit
