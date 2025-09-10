// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/Backends/Win32/Win32Window.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Win32 window implementation
// =============================================================================

#pragma once

#include "FrameKit/Window/Window.h"

#include <windows.h>

namespace FrameKit {

class Win32Window final : public IWindow {
public:
    explicit Win32Window(const WindowDesc& d);
    ~Win32Window() override;

    void poll() override;
    bool shouldClose() const override;
    void requestClose() override;

    void* nativeHandle() const override { return m_hwnd; }
    void* nativeDisplay() const override { return m_hinst; }
    uint32_t width()  const override { return m_w; }
    uint32_t height() const override { return m_h; }
    float contentScaleX() const override { return m_sx; }
    float contentScaleY() const override { return m_sy; }

    void setTitle(const std::string& t) override;
    void setVSync(bool e) override { m_vsync = e; }
    bool getVSync() const override { return m_vsync; }
    void setCursorMode(CursorMode m) override;

private:
    friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    HINSTANCE m_hinst = nullptr;
    HWND m_hwnd = nullptr;
    uint32_t m_w=0, m_h=0;
    float m_sx=1.f, m_sy=1.f;
    bool m_vsync=true;
    bool m_close=false;
};

} // namespace FrameKit