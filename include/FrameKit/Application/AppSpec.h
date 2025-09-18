// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Application/AppSpec.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines application specifications, optional settings, and command line arguments.
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"
#include "FrameKit/Window/IWindow.h"

#include <cstdint>
#include <filesystem>
#include <string>

namespace FrameKit {

    // ---------- Application mode ----------
    enum class AppMode : unsigned { Headless = 0, Windowed = 1 };

    // ---------- Window settings ----------
    struct WindowSettings {
        WindowAPI     api{ WindowAPI::Auto }; // Auto = first available
        std::string   title{ "FrameKit" };
        std::uint32_t width{ 1280 };
        std::uint32_t height{ 720 };
        bool          resizable{ true };
        bool          vsync{ true };
        bool          visible{ true };
        bool          highDPI{ true };               // if supported by backend

        bool GetVSync() const noexcept { return vsync; }
        void SetVSync(bool v) noexcept { vsync = v; }

        bool operator==(const WindowSettings&) const = default;
    };

    // ---------- Command line args ----------
    struct ApplicationCommandLineArgs {
        int    Count = 0;
        char** Args = nullptr;
        const char* operator[](int i) const noexcept {
            return (i >= 0 && i < Count) ? Args[i] : nullptr;
        }
    };

    // ---------- App spec ----------
    struct ApplicationSpecification {
        // Keep original field names for ABI/API stability
        std::string                Name = "FrameKit Application";
        std::filesystem::path      WorkingDirectory = {};
        ApplicationCommandLineArgs CommandLineArgs = {};
        AppMode                    Mode = AppMode::Windowed;
        WindowSettings             WinSettings = {};
        bool                       Master = false;  // optional, for multi-instance apps or IPC roles
    };

} // namespace FrameKit