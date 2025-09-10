// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Engine/Layer.cpp
// Author       : George Gil
// Created      : 2025-09-08
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Implements the Layer base class constructor.
// =============================================================================

#include "FrameKit/Engine/Layer.h"

namespace FrameKit
{
    Layer::Layer(const std::string& debugName)
        : m_DebugName(debugName)
    {
    }
}
