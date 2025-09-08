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
#include "FrameKit/Utilities/Time.h"

namespace FrameKit {
    class Application : public ApplicationBase {
    public:
        Application() = default;
        explicit Application(const ApplicationSpecification& spec) : ApplicationBase(spec) {}
        ~Application() override = default;

		// Lifecycle - Can be implemented by the client application
        bool Init() override { return true; }
		void Shutdown() override {}
		
		// sensible defaults; override as needed
		// NOTE: override only if you need to customise the execution loop
		// NOTE: Override only if you know what you are doing!
		bool OnUpdate(Timestep /*ts*/) override;
		void OnRender() override;// only called in windowed mode
		void OnEvent(Event& /*e*/) override;

		// For more advanced usage, override these hooks as needed:
		// void OnBeforePoll() override {}
		// void OnAfterPoll() override {}
		// void OnBeforeUpdate(double /*deltaTime*/) override {}
		// void OnAfterUpdate(double /*deltaTime*/) override {}
		// void OnBeforeRender() override {}
		// void OnAfterRender() override {}
		// void OnFrameEnd() override {}
		// void OnUnhandledEvent(Event& /*e*/) override {}
    };

    // Client must implement this to create their application instance.
    Application* CreateApplication(ApplicationCommandLineArgs args);

} // namespace FrameKit