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

#include <chrono>
#include <thread>
#include <memory>

namespace 
{
inline double NowSec() {
    using C = std::chrono::steady_clock;
    return std::chrono::duration<double>(C::now().time_since_epoch()).count();
}
inline void SleepSec(double s) { if (s > 0) std::this_thread::sleep_for(std::chrono::duration<double>(s)); }
}

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
        double target_dt = 0.0;
        double last = 0.0;
        unsigned long long frame = 0;
        bool closing = false;

        void SetupTarget(double max_fps) {
            target_dt = max_fps > 0.0 ? 1.0 / max_fps : 0.0;
            last = NowSec();
			frame = 0;
			closing = false;
        }

        bool PaceAndEndFrame(ApplicationBase& app, double frameStart) {
            if (target_dt > 0.0) {
                const double spent = NowSec() - frameStart;
                if (spent < target_dt) { SleepSec(target_dt - spent); }
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
            // TODO: Add max FPS to spec, pass it here. For now: uncapped.
            loop_.SetupTarget(0.0);
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

            const double t0 = NowSec();
            double dt = t0 - loop_.last;
            loop_.last = t0;

            app.OnBeforeUpdate(dt);
            if (!app.OnUpdate(dt)) { loop_.closing = true; }
            app.OnAfterUpdate(dt);

            if (!loop_.closing) {
                app.OnBeforeRender();
                app.OnRender();
                app.OnAfterRender();
                if (win_) win_->Swap();
            }

			stats_.dt = dt;
			stats_.frame = loop_.frame + 1; // next frame index
			return loop_.PaceAndEndFrame(app, t0);
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

            const double t0 = NowSec();
            double dt = t0 - loop_.last;
            loop_.last = t0;

            app.OnBeforeUpdate(dt);
            if (!app.OnUpdate(dt)) { loop_.closing = true; }
            app.OnAfterUpdate(dt);

			stats_.dt = dt;
			stats_.frame = loop_.frame + 1; // next frame index

			return loop_.PaceAndEndFrame(app, t0);
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
