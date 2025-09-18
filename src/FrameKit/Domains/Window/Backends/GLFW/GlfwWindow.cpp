// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/Backends/GLFW/GlfwWindow.cpp
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : GLFW window backend
// =============================================================================

#include "GlfwWindow.h"
#include "FrameKit/Window/WindowRegistry.h"
#include "FrameKit/Gfx/API/RendererConfig.h"
#include "FrameKit/Debug/Log.h"

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
#  define GLFW_EXPOSE_NATIVE_COCOA
#  include <GLFW/glfw3native.h>
#elif defined(__linux__)
#  define GLFW_EXPOSE_NATIVE_X11
#  define GLFW_EXPOSE_NATIVE_WAYLAND
#  include <GLFW/glfw3native.h>
#endif

#include <cassert>
#include <atomic>
#include <mutex>

namespace FrameKit {
    static std::mutex g_glfwMx;
    static std::atomic<int> g_glfwRef{ 0 };

    static void glfwInitRef() {
        std::lock_guard<std::mutex> lk(g_glfwMx);
        if (g_glfwRef.fetch_add(1, std::memory_order_acq_rel) == 0) {
            glfwSetErrorCallback([](int, const char* desc) {
                // hook to your logger if desired
                (void)desc;
            });
            if (!glfwInit()) {
                // hard fail; keep internal state consistent
                g_glfwRef.store(0, std::memory_order_release);
                assert(false && "glfwInit failed");
            }
        }
    }

    static void glfwTermRef() {
        std::lock_guard<std::mutex> lk(g_glfwMx);
        if (g_glfwRef.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            glfwTerminate();
        }
    }

    GlfwWindow::GlfwWindow(const WindowDesc& d) {
        glfwInitRef();

        // Do NOT set GLFW_CLIENT_API here. Caller decides (OpenGL vs NO_API).

        glfwWindowHint(GLFW_VISIBLE,   d.visible   ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, d.resizable ? GLFW_TRUE : GLFW_FALSE);

    #if defined(__APPLE__)
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, d.highDPI ? GLFW_TRUE : GLFW_FALSE);
    #else
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, d.highDPI ? GLFW_TRUE : GLFW_FALSE);
    #endif

        m_w = glfwCreateWindow((int)d.width, (int)d.height, d.title.c_str(), nullptr, nullptr);
        if (!m_w) { glfwTermRef(); assert(false && "glfwCreateWindow failed"); return; }

        int ww = 0, hh = 0;
        glfwGetWindowSize(m_w, &ww, &hh);
        m_wd = (uint32_t)ww; m_hd = (uint32_t)hh;

        float sx = 1.0f, sy = 1.0f;
        glfwGetWindowContentScale(m_w, &sx, &sy);
        m_sx = sx; m_sy = sy;

        setVSync(d.vsync);

        glfwSetWindowUserPointer(m_w, this);

        // Key input
        glfwSetKeyCallback(m_w, [](GLFWwindow* w, int key, int sc, int action, int mods) {
            if (auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w)) {
                if (self->onKey) self->onKey(RawKeyEvent{ key, sc, action, mods });
            }
        });
        // Mouse buttons
        glfwSetMouseButtonCallback(m_w, [](GLFWwindow* w, int button, int action, int mods) {
            if (auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w)) {
                if (self->onMouseBtn) self->onMouseBtn(RawMouseBtn{ button, action, mods });
            }
        });
        // Cursor motion
        glfwSetCursorPosCallback(m_w, [](GLFWwindow* w, double x, double y) {
            if (auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w)) {
                if (self->onMouseMove) self->onMouseMove(RawMouseMove{ x, y });
            }
        });

        // Scroll (high-resolution supported by GLFW)
        glfwSetScrollCallback(m_w, [](GLFWwindow* w, double dx, double dy) {
            if (auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w)) {
                if (self->onMouseWheel) self->onMouseWheel(RawMouseWheel{ dx, dy });
            }
        });

        // Resize (logical size)
        glfwSetWindowSizeCallback(m_w, [](GLFWwindow* w, int vw, int vh) {
            if (auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w)) {
                self->m_wd = (uint32_t)vw; self->m_hd = (uint32_t)vh;
                if (self->onResize) self->onResize(Resize{ vw, vh });
            }
        });

        // Content scale changes (per-monitor DPI switches)
        glfwSetWindowContentScaleCallback(m_w, [](GLFWwindow* w, float sx2, float sy2) {
            if (auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w)) {
                self->m_sx = sx2; self->m_sy = sy2;
            }
        });

        // Close request
        glfwSetWindowCloseCallback(m_w, [](GLFWwindow* w) {
            if (auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w)) {
                if (self->onCloseReq) self->onCloseReq(CloseReq{});
                self->m_close = true;
            }
            glfwSetWindowShouldClose(w, GLFW_TRUE);
        });

        if (d.visible) glfwShowWindow(m_w);
    }

    GlfwWindow::~GlfwWindow() {
        if (m_w) { glfwDestroyWindow(m_w); m_w = nullptr; }
        glfwTermRef();
    }

    void GlfwWindow::poll() { glfwPollEvents(); }

    bool GlfwWindow::shouldClose() const {
        return m_close || (m_w && glfwWindowShouldClose(m_w));
    }

    void GlfwWindow::requestClose() {
        m_close = true;
        if (m_w) glfwSetWindowShouldClose(m_w, GLFW_TRUE);
    }

    void* GlfwWindow::nativeHandle() const { return m_w; }

    void* GlfwWindow::nativeDisplay() const {
    #if defined(_WIN32)
        return nullptr; // HINSTANCE not exposed; nullptr acceptable by IWindow doc for GLFW
    #elif defined(__APPLE__)
        return glfwGetCocoaWindow(m_w); // NSWindow*
    #elif defined(__linux__)
        if (auto* dpy = glfwGetX11Display()) return dpy; // Display*
        return nullptr; // Wayland: nullptr
    #else
        return nullptr;
    #endif
    }

    uint32_t GlfwWindow::width() const { return m_wd; }
    uint32_t GlfwWindow::height() const { return m_hd; }
    float GlfwWindow::contentScaleX() const { return m_sx; }
    float GlfwWindow::contentScaleY() const { return m_sy; }

    void GlfwWindow::setTitle(const std::string& t) { glfwSetWindowTitle(m_w, t.c_str()); }

    void GlfwWindow::setVSync(bool enabled) {
        m_vsync = enabled; // renderer applies swap interval
    }

    bool GlfwWindow::getVSync() const { return m_vsync; }

    void GlfwWindow::setCursorMode(CursorMode m) {
        int mode = GLFW_CURSOR_NORMAL;
        if (m == CursorMode::Hidden) mode = GLFW_CURSOR_HIDDEN;
        if (m == CursorMode::Locked) mode = GLFW_CURSOR_DISABLED;
        glfwSetInputMode(m_w, GLFW_CURSOR, mode);
    }

    void GlfwWindow::Swap() {
        if (m_w) glfwSwapBuffers(m_w);
    }

    // ----- registration -----

    static void DeleteGlfw(IWindow* w) noexcept { delete w; }

    // static WindowPtr CreateGLFW(const WindowDesc& d) {
    //     auto* w = new GlfwWindow(d);
    //     WindowRegistry::Register(w, WindowAPI::GLFW, "GLFW");
    //     return WindowPtr(w, &DestroyAndUnregister<&DeleteGlfw>);
    // }

    static WindowPtr CreateGLFW_WithRenderCfg(const WindowDesc& d, const RendererConfig* rc) {
        // Decide API
        glfwDefaultWindowHints();
        if (rc && rc->api == GraphicsAPI::OpenGL) {
            FK_CORE_INFO("Creating GLFW window for OpenGL");
            const auto& gl = rc->gl;
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl.major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl.minor);
            glfwWindowHint(GLFW_OPENGL_PROFILE, gl.core ? GLFW_OPENGL_CORE_PROFILE
                                                        : GLFW_OPENGL_ANY_PROFILE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, gl.debug ? GLFW_TRUE : GLFW_FALSE);
        } else {
            FK_CORE_INFO("Creating GLFW window for Vulkan/No API");
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        auto* w = new GlfwWindow(d);
        WindowRegistry::Register(w, WindowAPI::GLFW, "GLFW");

        // If OpenGL, bind context and apply swap interval now.
        if (rc && rc->api == GraphicsAPI::OpenGL) {
            GLFWwindow* gw = static_cast<GLFWwindow*>(w->nativeHandle());
            glfwMakeContextCurrent(gw);
            // Renderer or caller may change it later; set initial swap interval here.
            if (rc->gl.swapInterval) glfwSwapInterval(d.vsync ? 1 : 0);
        }

        return WindowPtr(w, &DestroyAndUnregister<&DeleteGlfw>);
    }

    // explicit registrar callable from core
    extern "C" bool FrameKit_RegisterBackend_GLFW() {
        return RegisterWindowBackend(
            WindowAPI::GLFW,
            "GLFW",
            [](const WindowDesc& d, const RendererConfig* rc) {
                return CreateGLFW_WithRenderCfg(d, rc);
            },
            100);
    }

} // namespace FrameKit
