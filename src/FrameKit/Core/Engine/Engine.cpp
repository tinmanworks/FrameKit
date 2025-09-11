// =============================================================================
// Project: FrameKit
// File         : src/FrameKit/Engine/Engine.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//   Engine-owned entry. Delegates to client CreateApplication and runs the host loop.
// =============================================================================

#include "FrameKit/Core/Engine/Engine.h"
#include "FrameKit/Application/Application.h"
#include "FrameKit/Core/Engine/IAppHost.h"
#include "FrameKit/Debug/Log.h"

namespace FrameKit {

    int Engine(ApplicationBase& app) {
        auto host = MakeHost(app.GetSpec().Mode);
        if (!host) { app.Shutdown(); return 1; }
		if (!host->Init(app)) { app.Shutdown(); return 1; }

        while (host->Tick(app)) {}

        app.Shutdown();
        return 0;
    }

} // namespace FrameKit
