#pragma once
#include "FrameKit/Platform/Window.h"
#ifdef __OBJC__
@class NSWindow;
@class NSApplication;
#else
typedef void NSWindow;
typedef void NSApplication;
#endif

namespace FrameKit {
class CocoaWindow final : public IWindow {
public:
    explicit CocoaWindow(const WindowDesc& d);
    ~CocoaWindow() override;

    void poll() override;
    bool shouldClose() const override;
    void requestClose() override;

    void* nativeHandle() const override;
    void* nativeDisplay() const override { return nullptr; }
    uint32_t width() const override;
    uint32_t height() const override;
    float contentScaleX() const override;
    float contentScaleY() const override;

    void setTitle(const std::string& t) override;
    void setVSync(bool e) override { m_vsync = e; }
    bool getVSync() const override { return m_vsync; }
    void setCursorMode(CursorMode m) override;

private:
    NSApplication* m_app = nullptr;
    NSWindow* m_win = nullptr;
    uint32_t m_w=0, m_h=0;
    float m_sx=1.f, m_sy=1.f;
    bool m_vsync=true;
    bool m_close=false;
};
} // namespace FrameKit
