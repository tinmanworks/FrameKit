#pragma once
#include "GraphicsAPI.h"
#include <cstdint>

namespace FrameKit {

struct OpenGLOptions {
    int  major = 3;
    int  minor = 3;
    bool core  = true;
    bool debug = false;
    bool swapInterval = true; // apply vsync via glfwSwapInterval
};

struct VulkanOptions {
    uint32_t apiVersion = 0;     // e.g. VK_API_VERSION_1_3 (set by user)
    bool     enableValidation = false;
    // add ext lists later if needed
};

struct RendererConfig {
    GraphicsAPI api = GraphicsAPI::None;
    OpenGLOptions gl{};
    VulkanOptions  vk{};
};

} // namespace FrameKit
