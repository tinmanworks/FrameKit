// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Engine/Layer.h
// Author       : George Gil
// Created      : 2025-09-08
// Updated      : 2025-09-09
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//          Base interface for engine layers.Provides lifecycle hooks,
//          per - frame updates, event handling, and optional cyclic work.
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"
#include "FrameKit/Utilities/Time.h"

#include <string>

namespace FrameKit
{
	class Event;

    class Layer
    {
    public:
        // Optional debug name for the layer
        explicit Layer(const std::string& name = "Layer");

        virtual ~Layer() = default;

        // Lifecycle
        virtual void OnAttach() {}
        virtual void OnDetach() {}

		// Per-frame update in the main thread
        virtual void OnSyncUpdate(Timestep ts) {}

		// Called in the main thread if the layer supports rendering
		// Only in windowed mode
        virtual void OnRender() {}
        
		// Async update, called in a separate thread if the layer supports it
		// StartAsyncThread must be called by the application to enable this
		// Note: Thread safety must be ensured by the layer implementation
        virtual void OnAsyncUpdate() {}

		// Event handling
        virtual void OnEvent(Event& event) {}

        FK_NODISCARD const std::string& GetName() const noexcept { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };
}
