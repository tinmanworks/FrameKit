// =============================================================================
// Project: FrameKit
// File         : src/FrameKit/Core/EntryPoint.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//   Engine-owned entry point. Calls client-provided CreateApplication()
//   to obtain the app instance and runs its lifecycle. Modernized with RAII,
//   exception safety, and platform-guarded crash handling.
// =============================================================================

#include "FrameKit/Application/Application.h"


#include <exception>
#include <iostream>
#include <memory>



extern FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args);

int main(int argc, char** argv) {
    try {
        // Startup code: Create application instance and initialize everything
        std::unique_ptr<FrameKit::Application> app;
        {
            FrameKit::ApplicationCommandLineArgs args{ argc, argv };
            app.reset(FrameKit::CreateApplication(args));
        }
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
}