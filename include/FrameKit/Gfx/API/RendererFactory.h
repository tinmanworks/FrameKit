#pragma once
#include <memory>
#include "IRenderer.h"
#include "RendererConfig.h"

namespace FrameKit {
std::unique_ptr<IRenderer> CreateRenderer(const RendererConfig& cfg);
}
