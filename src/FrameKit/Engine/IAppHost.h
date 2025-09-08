// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Engine/IAppHost.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines the IAppHost interface for application hosts in the FrameKit framework.
//      Hosts manage the application lifecycle, including initialization,
//      the main run loop, and shutdown. They handle platform-specific details
// =============================================================================

#pragma once

#include "FrameKit/Application/AppSpec.h"

#include <memory>

namespace FrameKit {
    class ApplicationBase;

    struct HostStats { double ts = 0.0; unsigned long long frame = 0; };

    struct IAppHost {
        virtual ~IAppHost() = default;
		virtual bool Init(ApplicationBase& app) = 0;    // calls app.Init(); false => exit
		virtual bool Tick(ApplicationBase& app) = 0;    // one iteration of run loop; false => exit
        virtual void SignalClose() = 0;
        virtual HostStats Stats() const = 0;
    };

	std::unique_ptr<IAppHost> MakeHost(AppMode mode);
} // namespace FrameKit
