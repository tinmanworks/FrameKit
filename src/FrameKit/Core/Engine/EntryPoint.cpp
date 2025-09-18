// =============================================================================
// Project: FrameKit
// File         : src/FrameKit/Core/EntryPoint.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//   Entry point for FrameKit applications. Initializes the application,
//   runs its lifecycle, and handles cleanup.
// =============================================================================
#include "FrameKit/Application/Application.h"
#include "FrameKit/Core/Engine/Engine.h"
#include "FrameKit/Debug/Instrumentor.h"
#include "FrameKit/Debug/Log.h"

#include <exception>
#include <iostream>
#include <memory>

extern FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args);

int main(int argc, char** argv) {
    // TODO: allow configuring log level via command line args or env var
    // TODO: profiling name via app name or env var
    FK_PROFILE_BEGIN_SESSION("Startup", "FrameKitProfile.json");

    FrameKit::Log::Init(); // core="FrameKit", client="Application"
    // Optional: tune default levels or sinks here
     FrameKit::Log::GetCoreLogger()->set_level(FrameKit::LogLevel::Info);
     FrameKit::Log::GetClientLogger()->set_level(FrameKit::LogLevel::Trace);

    int exit_code = EXIT_SUCCESS;

    try {
        FrameKit::ApplicationCommandLineArgs args{ argc, argv };
        FK_CORE_INFO("Entry: argc={}", argc);

        std::unique_ptr<FrameKit::Application> app{ FrameKit::CreateApplication(args) };
        if (!app) {
            FK_CORE_ERROR("CreateApplication() returned null");
            exit_code = EXIT_FAILURE;
        }
        else {
            app->GetSpec().CommandLineArgs = args;
            FK_CORE_INFO("Application created: name='{}'", app->GetSpec().Name);

            // Host (Windowed or Headless) drives: Init -> Run Loop -> Shutdown
            exit_code = FrameKit::Engine(*app);
            FK_CORE_INFO("Engine exited with code {}", exit_code);
        }
    }
    catch (const std::exception& e) {
        FK_CORE_CRITICAL("Unhandled exception: {}", e.what());
        exit_code = EXIT_FAILURE;
    }
    catch (...) {
        FK_CORE_CRITICAL("Unhandled unknown exception");
        exit_code = EXIT_FAILURE;
    }

    FK_PROFILE_END_SESSION();
    FrameKit::Log::UninitClient();
    return exit_code;
}