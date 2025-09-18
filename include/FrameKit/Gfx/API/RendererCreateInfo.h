#pragma once
#include <cstdint>

namespace FrameKit {

class IWindow;      // forward-declare to avoid window headers here
struct RendererConfig;

struct RendererCreateInfo {
    IWindow*              window   = nullptr; // native handle lives here
    const RendererConfig* config   = nullptr; // chosen API + per-API opts
    bool                  vsync    = true;    // runtime toggle
    bool                  debug    = false;   // engine-wide debug
};

} // namespace FrameKit
