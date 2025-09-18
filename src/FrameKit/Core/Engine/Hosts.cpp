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
#include "FrameKit/Window/WindowEventBridge.h"
#include "FrameKit/Gfx/API/RendererConfig.h"
#include "FrameKit/Debug/Log.h"
#include "FrameKit/Debug/Instrumentor.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <memory>
#include <iostream>
#if __cpp_lib_format >= 202106L
#include <format>
#else
#include <fmt/format.h>
#endif

namespace FrameKit {
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
            FK_CORE_INFO("Loop target: {}", (max_fps > 0.0) ? std::to_string(max_fps) + " fps" : "uncapped");

        }

        bool PaceAndEndFrame(ApplicationBase& app) {
            FK_PROFILE_FUNCTION();
            if (target_dt.Seconds() > 0.0f) {
                auto spent = Timestep(std::chrono::duration<float>(steady::now() - frame_start));
                auto remain = target_dt - spent;
                if (remain.Seconds() > 0.0f) {
                    Sleep(remain);
                }
                else if (remain.Seconds() < -0.010f) {
                    FK_CORE_WARN("Frame over budget: {} ms", -remain.Milliseconds());
                }
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

            auto v = ListWindowBackends();

            if (v.empty()) {
                FK_CORE_WARN("Window Backends: none registered");
            }
            else {
                FK_CORE_INFO("Window Backends available: {}", v.size());
                for (const WindowAPIInfo& b : v) {
                    FK_CORE_INFO("Backend: api={} prio={}", ToString(b.id), b.priority);
                }
                FK_CORE_INFO("Requested API: {}", ToString(spec.WinSettings.api));
            }

            WindowDesc wd;
            wd.title = spec.WinSettings.title.empty() ? spec.Name : spec.WinSettings.title;
            wd.width = spec.WinSettings.width ? spec.WinSettings.width : 1280;
            wd.height = spec.WinSettings.height ? spec.WinSettings.height : 720;
            wd.vsync = spec.WinSettings.vsync;
            wd.visible = spec.WinSettings.visible;
            wd.resizable = spec.WinSettings.resizable;
            wd.highDPI = spec.WinSettings.highDPI;

            FK_CORE_INFO("Create window: '{}' {}x{} vsync={} resizable={} highDPI={}",
                wd.title, wd.width, wd.height, wd.vsync, wd.resizable, wd.highDPI);

            FK_CORE_INFO("RendererConfig: api={}", ToString(spec.GfxSettings.api));
            FK_CORE_INFO("OpenGL Options: major={} minor={} core={} debug={} swapInterval={}",
                spec.GfxSettings.gl.major,
                spec.GfxSettings.gl.minor,
                spec.GfxSettings.gl.core ? "true" : "false",
                spec.GfxSettings.gl.debug ? "true" : "false",
                spec.GfxSettings.gl.swapInterval ? "true" : "false");
                
            // pick best available backend
            WindowPtr w = CreateWindow(spec.WinSettings.api, wd, &spec.GfxSettings);
            if (!w) {
                FK_CORE_ERROR("CreateWindow failed for api={}", ToString(spec.WinSettings.api));
                return false;
            }
            win_ = std::move(w);
            BindWindowToGlobalEvents(*win_);
            FK_CORE_TRACE("Window created and event bridge bound");

            const bool ok = app.Init();
            if (!ok) {
                FK_CORE_ERROR("Application Init failed");
            }
            else {
                FK_CORE_INFO("Application Init ok");
            }
            return ok;
        }

        bool Tick(ApplicationBase& app) override {
            FK_PROFILE_FUNCTION();
            if (loop_.closing) return false;

            app.OnBeforePoll();
            if (!win_) {
                FK_CORE_ERROR("Tick: window invalid");
                app.OnAfterPoll();
                loop_.closing = true;
                return false;
            }

            win_->poll();
            if (win_->shouldClose()) {
                FK_CORE_INFO("Window requested close");
                app.OnAfterPoll();
                loop_.closing = true;
                return false;
            }
            app.OnAfterPoll();

            loop_.frame_start = CommonLoop::steady::now();
            loop_.clock.Tick();
            Timestep ts = loop_.clock.Delta();
            if (ts.Seconds() <= 0.0f) ts = Timestep(1.0f / 60.0f);

            app.OnBeforeUpdate(ts);
            if (!app.OnUpdate(ts)) {
                FK_CORE_INFO("App requested shutdown from OnUpdate");
                loop_.closing = true;
            }
            app.OnAfterUpdate(ts);

            if (!loop_.closing) {
                app.OnBeforeRender();
                app.OnRender();
                app.OnAfterRender();
            }

            stats_.ts = ts.Seconds();
            stats_.frame = loop_.frame + 1;

            if (win_) win_->Swap();

            return loop_.PaceAndEndFrame(app);
        }

        void SignalClose() override {
            FK_CORE_INFO("SignalClose");
            loop_.closing = true;
            if (win_) win_->requestClose();
        }
        HostStats Stats() const override { return stats_; }
    };

    // ---------------- Headless host ----------------
    class HeadlessHost final : public IAppHost {
        CommonLoop loop_;
        HostStats stats_{};
    public:
        bool Init(ApplicationBase& app) override {
            loop_.SetupTarget(0.0);
            const bool ok = app.Init();
            if (!ok) FK_CORE_ERROR("Headless: Application Init failed");
            else     FK_CORE_INFO("Headless: Application Init ok");
            return ok;
        }

        bool Tick(ApplicationBase& app) override {
            if (loop_.closing) return false;

            app.OnBeforePoll();
            app.OnAfterPoll();

            loop_.frame_start = CommonLoop::steady::now();
            loop_.clock.Tick();
            Timestep ts = loop_.clock.Delta();

            app.OnBeforeUpdate(ts);
            if (!app.OnUpdate(ts)) {
                FK_CORE_INFO("Headless: App requested shutdown from OnUpdate");
                loop_.closing = true;
            }
            app.OnAfterUpdate(ts);

            stats_.ts = ts.Seconds();
            stats_.frame = loop_.frame + 1;

            return loop_.PaceAndEndFrame(app);
        }

        void SignalClose() override { FK_CORE_INFO("Headless SignalClose"); loop_.closing = true; }
        HostStats Stats() const override { return stats_; }
    };

    // Factory used by Engine
    std::unique_ptr<IAppHost> MakeHost(AppMode mode) {
        FK_PROFILE_FUNCTION();
        if (mode == AppMode::Headless) { FK_CORE_INFO("MakeHost: mode=Headless"); }
        else { FK_CORE_INFO("MakeHost: mode=Windowed"); }
        if (mode == AppMode::Windowed) return std::make_unique<WindowedHost>();
        return std::make_unique<HeadlessHost>();
    }


} // namespace FrameKit
