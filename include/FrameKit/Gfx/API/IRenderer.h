#pragma once
#include <cstdint>

namespace FrameKit {

struct RendererCreateInfo;

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool Init(const RendererCreateInfo& ci) = 0;
    virtual void Resize(uint32_t w, uint32_t h) = 0;
    virtual void Draw() = 0;
    virtual void Shutdown() = 0;
};

} // namespace FrameKit
