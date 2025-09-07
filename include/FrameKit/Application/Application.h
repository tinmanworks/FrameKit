// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Application/Application.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines the Application class for the FrameKit framework. Inherits from
//      ApplicationBase. Applications should derive from this to implement their
//      own Init/Shutdown and custom logic.
// =============================================================================

#pragma once

namespace FrameKit {

    struct ApplicationCommandLineArgs {
        int Count = 0;
        char** Args = nullptr;

        ApplicationCommandLineArgs() = default;

        ApplicationCommandLineArgs(int count, char** args)
            : Count(count), Args(args) {}
    };

    class Application {
    public:
        Application() = default;
        virtual ~Application() = default;

        // Initialize the application. Return true on success.
        virtual bool Init(const ApplicationCommandLineArgs& args) { return true; }

        // Shutdown and cleanup resources.
        virtual void Shutdown() {}

        // Main update loop. Called every frame.
        virtual void Update(float deltaTime) {}

        // Handle application-specific events.
        virtual void OnEvent(int eventType, void* eventData) {}
    };

    // Client must implement this to create their application instance.
    Application* CreateApplication(ApplicationCommandLineArgs args);
} // namespace FrameKit