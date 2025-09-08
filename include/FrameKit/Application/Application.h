// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Application/Application.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines the Application class for the FrameKit framework. Inherits from
//      ApplicationBase. Applications should derive from this to implement their
//      own Init/Shutdown and custom logic.
// =============================================================================

#pragma once

#include "FrameKit/Application/ApplicationBase.h"

namespace FrameKit {
    class Application : public ApplicationBase {
    public:
        Application() = default;
        explicit Application(const ApplicationSpecification& spec) : ApplicationBase(spec) {}
        ~Application() override = default;

		// sensible defaults; override as needed
        bool Init() override { return true; }
		void Shutdown() override {}
        bool OnUpdate(double /*deltaTime*/) override { return false; }
		void OnRender() override {}
		void OnEvent(Event& /*e*/) override {}
    };

    // Client must implement this to create their application instance.
    Application* CreateApplication(ApplicationCommandLineArgs args);

} // namespace FrameKit