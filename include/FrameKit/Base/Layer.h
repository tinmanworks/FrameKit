// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Base/Layer.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Base interface for engine layers. Provides lifecycle hooks,
//      per-frame updates, event handling, and optional cyclic work.
// =============================================================================

#pragma once

#include "FrameKit/Base/Base.h"
#include "FrameKit/Utils/Timestep.h"
#include "FrameKit/Events/Event.h"

#include <string>

namespace FrameKit
{
    class Layer
    {
    public:
        // Optional debug name for the layer
        explicit Layer(const std::string& name = "Layer");

        virtual ~Layer() = default;

        // Lifecycle
        virtual void OnAttach() {}
        virtual void OnDetach() {}

        // Per-frame update
        virtual void OnUpdate(Timestep ts) {}

        // ImGui rendering
        virtual void OnImGuiRender() {}

        // Event handling
        virtual void OnEvent(Event& event) {}

        // Optional cyclic work (e.g., background tasks)
        virtual void OnCyclic() {}

        [[nodiscard]] const std::string& GetName() const noexcept { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };
}
