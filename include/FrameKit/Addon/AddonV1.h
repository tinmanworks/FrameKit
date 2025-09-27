// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/AddonV1.h
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-09-28
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Interface for FrameKit addons version 1.
// =============================================================================

#pragma once

#include "AddonBase.h"

namespace FrameKit {
    struct IAddonV1 : public IAddonBase {
        virtual ~IAddonV1() override = default;

        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnRender() = 0;
        virtual void OnCyclic() = 0;
    };
}