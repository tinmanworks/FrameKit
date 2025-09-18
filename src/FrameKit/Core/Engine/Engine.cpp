// =============================================================================
// Project: FrameKit
// File         : src/FrameKit/Engine/Engine.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//   Engine-owned entry. Delegates to client CreateApplication and runs the host loop.
// =============================================================================

#include "FrameKit/Core/Engine/Engine.h"
#include "FrameKit/Application/Application.h"
#include "FrameKit/Core/Engine/IAppHost.h"
#include "FrameKit/Debug/Log.h"

namespace FrameKit {

    int Engine(ApplicationBase& app)  {
        FK_CORE_INFO("Engine start: app='{}' mode={}", app.GetSpec().Name, static_cast<int>(app.GetSpec().Mode));

        auto host = MakeHost(app.GetSpec().Mode);
        if (!host) {
            if (app.GetSpec().Mode == AppMode::Headless) { FK_CORE_ERROR("MakeHost failed: mode=Headless"); }
            else { FK_CORE_ERROR("MakeHost failed: mode=Windowed"); }
            
            app.Shutdown();
            FK_CORE_INFO("Engine stop with code 1");
            return 1;
        }

        if (!host->Init(app)) {
            FK_CORE_ERROR("Host.Init failed");
            app.Shutdown();
            FK_CORE_INFO("Engine stop with code 1");
            return 1;
        }
        FK_CORE_TRACE("Host.Init done");

        std::uint64_t frames = 0;
        while (host->Tick(app)) {
            ++frames;
            FK_CORE_TRACE("Tick {}", frames);
        }
        FK_CORE_INFO("Host loop exit: frames={}", frames);

        app.Shutdown();
        FK_CORE_INFO("Engine stop with code 0");
        return 0;
    }

} // namespace FrameKit
