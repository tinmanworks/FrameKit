// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Engine/Host.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Implements application hosts (Windowed and Headless) for the FrameKit framework.
//      Hosts manage the application lifecycle, including initialization,
//      the main run loop, and shutdown. They handle platform-specific details
// =============================================================================


#include "IAppHost.h"
#include "FrameKit/Application/Application.h"
#include "FrameKit/Utilities/Time.h"

#include <chrono>
#include <thread>
#include <memory>
#include <iostream>

namespace FrameKit {

    // Minimal internal window port (no public exposure)
    struct IWindow {
        virtual ~IWindow() = default;
        virtual bool Pump() = 0;           // false => close requested
        virtual void Swap() = 0;
        virtual void RequestClose() = 0;
        virtual unsigned Width()  const = 0;
        virtual unsigned Height() const = 0;
    };

    struct NullWindow final : IWindow {
        bool Pump() override { return true; }
        void Swap() override {}
        void RequestClose() override {}
        unsigned Width()  const override { return 0; }
        unsigned Height() const override { return 0; }
    };

    // TODO: replace with real platform backend
    static std::unique_ptr<IWindow> MakeWindow(unsigned, unsigned, bool) {
        return std::make_unique<NullWindow>();
    }

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
        std::unique_ptr<IWindow> win_;
        CommonLoop loop_;
        HostStats stats_{};
    public:
        bool Init(ApplicationBase& app) override {
            const auto& spec = app.GetSpec();
            loop_.SetupTarget(0.0); // uncapped for now; wire max FPS from spec later
            win_ = MakeWindow(1280, 720, spec.VSync);
            return app.Init();
        }

        bool Tick(ApplicationBase& app) override {
            if (loop_.closing) return false;

            app.OnBeforePoll();
            if (!win_ || !win_->Pump()) {
                app.OnAfterPoll();
                loop_.closing = true;
                return false;
            }
            app.OnAfterPoll();

            loop_.frame_start = CommonLoop::steady::now();
            loop_.clock.Tick();                         // updates internal delta/elapsed
            Timestep ts = loop_.clock.Delta();
			ts = (ts.Seconds() > 0.0f) ? ts : Timestep(1.0f / 60.0f); // clamp to 60 FPS min
            app.OnBeforeUpdate(ts);
            if (!app.OnUpdate(ts)) { loop_.closing = true; }
            app.OnAfterUpdate(ts);

            if (!loop_.closing) {
                app.OnBeforeRender();
                app.OnRender();
                app.OnAfterRender();
                if (win_) win_->Swap();
            }

			stats_.ts = ts.Seconds();                   // keep HostStats as seconds scalar
            stats_.frame = loop_.frame + 1;             // next frame index
            
            return loop_.PaceAndEndFrame(app);
        }

        void SignalClose() override { loop_.closing = true; if (win_) win_->RequestClose(); }
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
        if (mode == AppMode::Windowed) return std::make_unique<WindowedHost>();
        return std::make_unique<HeadlessHost>();
    }


} // namespace FrameKit
