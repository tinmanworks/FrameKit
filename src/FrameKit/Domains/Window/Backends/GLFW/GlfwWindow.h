// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/Backends/GLFW/GlfwWindow.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : GLFW window backend
// =============================================================================

#pragma once

#include "FrameKit/Window/Window.h"

struct GLFWwindow;

namespace FrameKit {

class GlfwWindow final : public IWindow {
public:
    explicit GlfwWindow(const WindowDesc& d);
    ~GlfwWindow() override;

    void poll() override;
    bool shouldClose() const override;
    void requestClose() override;

    void* nativeHandle() const override;
    void* nativeDisplay() const override;
    uint32_t width() const override;
    uint32_t height() const override;
    float contentScaleX() const override;
    float contentScaleY() const override;

    void setTitle(const std::string& t) override;
    void setVSync(bool enabled) override;
    bool  getVSync() const override;
    void setCursorMode(CursorMode m) override;

private:
    GLFWwindow* m_w = nullptr;
    uint32_t m_wd = 0, m_hd = 0;
    float m_sx = 1.0f, m_sy = 1.0f;
    bool m_vsync = true;
};

} // namespace FrameKit
