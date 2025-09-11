// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Application/AppSpec.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines application specifications, optional settings, and command line arguments.
// =============================================================================

#pragma once

#include <string>

namespace FrameKit {

    // ---------- Application mode ----------
    enum class AppMode : unsigned { Headless = 0, Windowed = 1 };


	// ---------- Command line args ----------
    struct ApplicationCommandLineArgs {
        int    Count = 0;
        char** Args = nullptr;
        const char* operator[](int i) const noexcept { return (i >= 0 && i < Count) ? Args[i] : nullptr; }
    };

    // ---------- App spec ----------
    struct ApplicationSpecification {
        std::string                Name             = "FrameKit Application";
        std::string                WorkingDirectory = {};
        ApplicationCommandLineArgs CommandLineArgs  = {};
        AppMode                    Mode             = AppMode::Windowed;
        bool                       VSync            = true;
		bool                       Master           = false;    // optional, for multi-instance apps or IPC roles
    };

} // namespace FrameKit