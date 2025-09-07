/*
 * Project: FrameKit
 * File: Layer.cpp
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   Implements the Layer base class constructor.
 */

#include "fkpch.h"
#include "FrameKit/Core/Layer.h"

namespace FrameKit
{
    Layer::Layer(const std::string& debugName)
        : m_DebugName(debugName)
    {
    }
}
