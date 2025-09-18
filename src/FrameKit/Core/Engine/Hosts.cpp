// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Engine/Host.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Implements application hosts (Windowed and Headless) for the FrameKit framework.
//      Hosts manage the application lifecycle, including initialization,
//      the main run loop, and shutdown. They handle platform-specific details
// =============================================================================

#include "IAppHost.h"
#include "FrameKit/Application/Application.h"
#include "FrameKit/Utilities/Time.h"
#include "FrameKit/Window/IWindow.h"
#include "FrameKit/Debug/Log.h"
#include "FrameKit/Debug/Instrumentor.h"
#include "FrameKit/Window/WindowEventBridge.h"

#include <chrono>
#include <thread>
#include <memory>
#include <iostream>

namespace FrameKit {
    // use the real window system
    using FrameKit::IWindow;
    using FrameKit::WindowPtr;
    
    static void NoopDelete(IWindow*) noexcept {}

    // Shared loop utilities
    struct CommonLoop {
        using steady = std::chrono::steady_clock;

        Timestep                target_dt{};   // 0 => uncapped
        steady::time_point      frame_start{};
        Clock                   clock{};       // provides per-frame delta and total
        unsigned long long      frame = 0;
        bool                    closing = false;

        void SetupTarget(double max_fps) {
            target_dt = (max_fps > 0.0) ? Timestep(static_cast<float>(1.0 / max_fps)) : Timestep{};
            frame = 0;
            closing = false;
            frame_start = steady::now();
            // clock constructed with its own start; first Tick() sets a valid delta
        }

        bool PaceAndEndFrame(ApplicationBase& app) {
			FK_PROFILE_FUNCTION();
            if (target_dt.Seconds() > 0.0f) {
                auto spent = Timestep(std::chrono::duration<float>(steady::now() - frame_start));
                auto remain = target_dt - spent;
                if (remain.Seconds() > 0.0f) Sleep(remain);
            }
            ++frame;
            app.OnFrameEnd();
            return !closing;
        }
    };

    // ---------------- Windowed host ----------------
    class WindowedHost final : public IAppHost {
        WindowPtr win_{ nullptr, &NoopDelete };
        CommonLoop loop_;
        HostStats stats_{};
    public:
        bool Init(ApplicationBase& app) override {
            FK_PROFILE_FUNCTION();
            const auto& spec = app.GetSpec();
            loop_.SetupTarget(0.0); // uncapped for now; wire max FPS from spec later

            //RegisterBuiltInWindowBackends();
			// optionally load window backends from plugins
            // LoadWindowPluginsFrom(std::filesystem::path(spec.WorkingDirectory) / "plugins");

			// Debug: list available backends
            FK_CORE_INFO("Window Backends:");
            for (auto& b : ListWindowBackends()) {
                FK_CORE_INFO("Backend ID:{} -> {}, Priority: {}", (int)b.id, b.name, b.priority);
            }

            WindowDesc wd;
            wd.title = spec.WinSettings.title.empty() ? spec.Name : spec.WinSettings.title;
            wd.width = spec.WinSettings.width ? spec.WinSettings.width : 1280;
            wd.height = spec.WinSettings.height ? spec.WinSettings.height : 720;
            wd.vsync = spec.WinSettings.vsync;
            wd.visible = spec.WinSettings.visible;
            wd.resizable = spec.WinSettings.resizable;
            wd.highDPI = spec.WinSettings.highDPI;

            // pick best available backend
            WindowPtr w = CreateWindow(spec.WinSettings.api, wd);
            if (!w) return false;
            win_ = std::move(w);
            BindWindowToGlobalEvents(*win_);
            return app.Init();
        }

        bool Tick(ApplicationBase& app) override {
            FK_PROFILE_FUNCTION();
            if (loop_.closing) return false;

            app.OnBeforePoll();
            if (!win_) { app.OnAfterPoll(); loop_.closing = true; return false; }
            win_->poll();
            if (win_->shouldClose()) { app.OnAfterPoll(); loop_.closing = true; return false; }
            app.OnAfterPoll();

            loop_.frame_start = CommonLoop::steady::now();
            loop_.clock.Tick();
            Timestep ts = loop_.clock.Delta();
            ts = (ts.Seconds() > 0.0f) ? ts : Timestep(1.0f / 60.0f);

            app.OnBeforeUpdate(ts);
            if (!app.OnUpdate(ts)) loop_.closing = true;
            app.OnAfterUpdate(ts);

            if (!loop_.closing) {
                app.OnBeforeRender();
                app.OnRender();
                app.OnAfterRender();
                // renderer applies vsync on present; window just stores the flag
            }

            stats_.ts = ts.Seconds();
            stats_.frame = loop_.frame + 1;

            return loop_.PaceAndEndFrame(app);
        }


        void SignalClose() override { loop_.closing = true; if (win_) win_->requestClose(); }
        HostStats Stats() const override { return stats_; }
    };

    // ---------------- Headless host ----------------
    class HeadlessHost final : public IAppHost {
        CommonLoop loop_;
        HostStats stats_{};
    public:
        bool Init(ApplicationBase& app) override {
            loop_.SetupTarget(0.0); // uncapped by default
            return app.Init();
        }

        bool Tick(ApplicationBase& app) override {
            if (loop_.closing) return false;

            app.OnBeforePoll();
            app.OnAfterPoll();

            loop_.frame_start = CommonLoop::steady::now();
            loop_.clock.Tick();
            Timestep ts = loop_.clock.Delta();

            app.OnBeforeUpdate(ts);
            if (!app.OnUpdate(ts)) { loop_.closing = true; }
            app.OnAfterUpdate(ts);
            
            stats_.ts = ts.Seconds();
            stats_.frame = loop_.frame + 1;

			return loop_.PaceAndEndFrame(app);
        }

        void SignalClose() override { loop_.closing = true; }
        HostStats Stats() const override { return stats_; }
    };

    // Factory used by Engine
    std::unique_ptr<IAppHost> MakeHost(AppMode mode) {
        FK_PROFILE_FUNCTION();
        if (mode == AppMode::Windowed) return std::make_unique<WindowedHost>();
        return std::make_unique<HeadlessHost>();
    }


} // namespace FrameKit
