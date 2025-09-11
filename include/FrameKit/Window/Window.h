// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Window/Window.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Window abstraction and backend registry
// =============================================================================

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace FrameKit {

    struct WindowDesc {
        std::string title = "FrameKit";
        uint32_t width = 1280, height = 720;
        bool resizable = true, vsync = true, visible = true, highDPI = true;
    };

    enum class WindowBackend : int { Auto = 0, GLFW = 1, Win32 = 2, Cocoa = 3 };
    enum class CursorMode { Normal, Hidden, Locked };

    inline const char* ToString(WindowBackend b) {
        switch (b) {
        case WindowBackend::Auto:  return "Auto";
        case WindowBackend::GLFW:  return "GLFW";
        case WindowBackend::Win32: return "Win32";
        case WindowBackend::Cocoa: return "Cocoa";
        default:                   return "Unknown";
        }
    }

    // Raw input structs (backend → window layer)
    struct RawKeyEvent { int key, scancode, action, mods; };   // action: 0=release,1=press,2=repeat
    struct RawMouseBtn { int button, action, mods; };          // action: 0=release,1=press
    struct RawMouseMove { double x, y; };
    struct RawMouseWheel { double dx, dy; };
    struct Resize { int width, height; };
    struct CloseReq {};

    class IWindow {
    public:
        virtual ~IWindow() = default;

        virtual void poll() = 0;
        virtual bool shouldClose() const = 0;
        virtual void requestClose() = 0;

        virtual void* nativeHandle() const = 0;   // HWND / NSWindow* / GLFWwindow*
        virtual void* nativeDisplay() const = 0;  // HINSTANCE / Display* / nullptr
        virtual uint32_t width() const = 0;
        virtual uint32_t height() const = 0;
        virtual float contentScaleX() const = 0;
        virtual float contentScaleY() const = 0;

        virtual void setTitle(const std::string& t) = 0;
        virtual void setVSync(bool enabled) = 0;  // stored flag; renderer applies swap interval
        virtual bool  getVSync() const = 0;
        virtual void setCursorMode(CursorMode m) = 0;

        using KeyCallback = void(*)(const RawKeyEvent&);
        using MouseBtnCb = void(*)(const RawMouseBtn&);
        using MouseMoveCb = void(*)(const RawMouseMove&);
        using MouseWheelCb = void(*)(const RawMouseWheel&);
        using ResizeCb = void(*)(const Resize&);
        using CloseReqCb = void(*)(const CloseReq&);

        KeyCallback   onKey = nullptr;
        MouseBtnCb    onMouseBtn = nullptr;
        MouseMoveCb   onMouseMove = nullptr;
        MouseWheelCb  onMouseWheel = nullptr;
        ResizeCb      onResize = nullptr;
        CloseReqCb    onCloseReq = nullptr;
    };

    using WindowPtr = std::unique_ptr<IWindow, void(*)(IWindow*)>;
    using CreateWindowFn = std::function<WindowPtr(const WindowDesc&)>;

    // registry
    bool RegisterWindowBackend(WindowBackend id, std::string_view name, CreateWindowFn createfn, int priority); // priority: higher = preferred

    struct WindowBackendInfo { WindowBackend id; std::string name; int priority; };

    std::vector<WindowBackendInfo> ListWindowBackends();
    WindowPtr CreateWindow(WindowBackend id, const WindowDesc& d);

} // namespace FrameKit