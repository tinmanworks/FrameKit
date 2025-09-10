// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Events/ApplicationEvent.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Application-level events (window lifecycle & app ticks)
// =============================================================================

#pragma once
#include "FrameKit/Events/Event.h"

namespace FrameKit {

class AppTickEvent final : public Event {
public: EVENT_CLASS_TYPE(AppTick)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppUpdateEvent final : public Event {
public: EVENT_CLASS_TYPE(AppUpdate)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppRenderEvent final : public Event {
public: EVENT_CLASS_TYPE(AppRender)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

} // namespace FrameKit

