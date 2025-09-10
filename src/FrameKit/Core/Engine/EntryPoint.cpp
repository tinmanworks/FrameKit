// =============================================================================
// Project: FrameKit
// File         : src/FrameKit/Core/EntryPoint.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//   Entry point for FrameKit applications. Initializes the application,
//   runs its lifecycle, and handles cleanup.
// =============================================================================
#include "FrameKit/Application/Application.h"
#include "FrameKit/Core/Engine/Engine.h"
#include "FrameKit/Debug/Instrumentor.h"

#include <exception>
#include <iostream>
#include <memory>

extern FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args);

int main(int argc, char** argv) {
#if FK_PROFILE
    FK_PROFILE_BEGIN_SESSION("Startup", "FrameKitProfile.json");
#endif

    try {
        FrameKit::ApplicationCommandLineArgs args{ argc, argv };

        std::unique_ptr<FrameKit::Application> app{ FrameKit::CreateApplication(args) };
        if (!app) {
            std::cerr << "Failed to create application instance." << std::endl;
            return EXIT_FAILURE;
        }

        // pass args into spec for app access like maybe at Init time
        app->GetSpec().CommandLineArgs = args;

        // Host (Windowed or Headless) drives: Init -> Run Loop -> Shutdown
        const int code = FrameKit::Engine(*app);
        return code;

    }
    catch (const std::exception& e) {
        // Log the exception details
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unhandled unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
#if FK_PROFILE
    FK_PROFILE_END_SESSION();
#endif
    return 0;
}