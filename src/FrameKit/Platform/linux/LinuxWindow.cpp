/*
 * Project: FrameKit
 * File: LinuxWindow.cpp
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   GLFW-based Window implementation for Linux.
 */

// TODO : Perhaps merge LinuxWindow and WindowsWindow into a single GLFWWindow class?
// TODO : Move GLFW and glad stuff to optional library module which can be excluded if you don't need OpenGL or GLFW or any window for that matter?
#include "FrameKit/Platform/linux/LinuxWindow.h"

#include "FrameKit/Debug/Log.h"
#include "FrameKit/Base/Assert.h"

#include "FrameKit/Events/ApplicationEvent.h"
#include "FrameKit/Events/MouseEvent.h"
#include "FrameKit/Events/KeyEvent.h"

#include "FrameKit/Interface/KeyCodes.h"
#include "FrameKit/Interface/MouseCodes.h"

 // GLAD must be included BEFORE GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <cstdlib>   // std::getenv
#include <string>

#if defined(FK_HAS_STB_IMAGE)
#include "stb_image.h"
#endif

namespace FrameKit {

    static std::uint8_t s_GLFWWindowCount = 0;

    static std::atomic<bool> s_Probing = false;
    static void GLFWErrorCallback(int error, const char* description) {
        if (s_Probing.load())
            FK_CORE_INFO("GLFW (probe) {}: {}", error, description);
        else
            FK_CORE_ERROR("GLFW Error ({}): {}", error, description);
    }

#if defined(FK_DEBUG)
    // GL debug callback (core since 4.3). No KHR_debug extension dependency.
    static void APIENTRY GLDebugCallback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/,
        GLenum severity, GLsizei, const GLchar* message, const void*) {
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:   FK_CORE_ERROR("GL: {}", message); break;
        case GL_DEBUG_SEVERITY_MEDIUM: FK_CORE_WARN("GL: {}", message); break;
        case GL_DEBUG_SEVERITY_LOW:    FK_CORE_INFO("GL: {}", message); break;
        default:                       FK_CORE_TRACE("GL: {}", message); break;
        }
    }
#endif

    template <typename T>
    static inline T CastToCode(int value) noexcept {
        if constexpr (std::is_enum_v<T>) {
            using U = std::underlying_type_t<T>;
            return static_cast<T>(static_cast<U>(value));
        }
        else {
            return static_cast<T>(value);
        }
    }

    LinuxWindow::LinuxWindow(const WindowProps& props) {
        FK_PROFILE_FUNCTION();
        Init(props);
    }

    LinuxWindow::~LinuxWindow() {
        FK_PROFILE_FUNCTION();
        Shutdown();
    }

    void LinuxWindow::Init(const WindowProps& props) {
        FK_PROFILE_FUNCTION();

        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        FK_CORE_INFO("Creating window {} ({} x {})", m_Data.Title, m_Data.Width, m_Data.Height);

        if (s_GLFWWindowCount == 0) {
            FK_PROFILE_SCOPE("glfwInit");
            if (!glfwInit()) {
                FK_CORE_CRITICAL("GLFW initialization failed. Aborting window creation.");
                // bail out cleanly: throw or return from Init() with a failure flag
                // e.g., throw std::runtime_error("glfwInit failed");
                return; // if your design allows it; otherwise throw
            }
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        // Decide GLX vs EGL preference
        const bool onX11 = std::getenv("DISPLAY") != nullptr;
        const bool onWayland = std::getenv("WAYLAND_DISPLAY") != nullptr;

        struct Ver { int maj, min; };
        // Highest -> lowest
        static constexpr Ver candidates[] = {
            {4,6},{4,5},{4,4},{4,3},{4,2},{4,1},{4,0},
            {3,3},{3,2},{3,1},{3,0},
            {2,1}
        };

        GLFWwindow* created = nullptr;
        bool usedEGL = false;
        int requestedMaj = 0, requestedMin = 0;

        auto tryCreate = [&](int maj, int min, bool useEGL)->GLFWwindow* {
            glfwDefaultWindowHints(); // reset every attempt

            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, maj);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, min);

            // Core profile for >= 3.2; legacy for 2.1
            if (maj > 3 || (maj == 3 && min >= 2))
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(FK_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

            // Conservative FB config: widely supported
            glfwWindowHint(GLFW_SAMPLES, 0);    // no MSAA in default FB
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
            glfwWindowHint(GLFW_RED_BITS, 8);
            glfwWindowHint(GLFW_GREEN_BITS, 8);
            glfwWindowHint(GLFW_BLUE_BITS, 8);
            glfwWindowHint(GLFW_ALPHA_BITS, 8);
            glfwWindowHint(GLFW_DEPTH_BITS, 24);
            glfwWindowHint(GLFW_STENCIL_BITS, 8);

            // Creation API selection
            glfwWindowHint(GLFW_CONTEXT_CREATION_API,
                useEGL ? GLFW_EGL_CONTEXT_API : GLFW_NATIVE_CONTEXT_API);

            return glfwCreateWindow(
                static_cast<int>(m_Data.Width),
                static_cast<int>(m_Data.Height),
                m_Data.Title.c_str(),
                nullptr, nullptr
            );
            };

        // Probe: prefer GLX on X11, EGL on Wayland; if unknown, try both.
        s_Probing = true;
        for (const auto& v : candidates) {
            requestedMaj = v.maj; requestedMin = v.min;

            if (onX11) {
                if ((created = tryCreate(v.maj, v.min, /*useEGL=*/false))) { usedEGL = false; break; }
            }
            else if (onWayland) {
                if ((created = tryCreate(v.maj, v.min, /*useEGL=*/true))) { usedEGL = true;  break; }
            }
            else {
                if ((created = tryCreate(v.maj, v.min, /*useEGL=*/false))) { usedEGL = false; break; }
                if ((created = tryCreate(v.maj, v.min, /*useEGL=*/true))) { usedEGL = true;  break; }
            }
        }
        s_Probing = false;

        if (!created) {
            FK_CORE_CRITICAL("Failed to create GLFW window!");
            // Do NOT touch s_GLFWWindowCount; nothing was created.
            return; // or throw
        }
        m_Window = created;
        ++s_GLFWWindowCount;

        // Make context current BEFORE any GL calls (incl. SwapInterval)
        glfwMakeContextCurrent(m_Window);

        // Load GL function pointers via GLAD
        {
            FK_PROFILE_SCOPE("gladLoadGLLoader");
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                FK_CORE_CRITICAL("Failed to initialize GLAD!");

                // Clean up gracefully so later code doesn't hit GLFW after terminate.
                // Detach current context (optional hygiene)
                if (glfwGetCurrentContext() == m_Window)
                    glfwMakeContextCurrent(nullptr);

                glfwDestroyWindow(m_Window);
                m_Window = nullptr;

                FK_CORE_ASSERT(s_GLFWWindowCount > 0, "GLFW window count underflow!");
                --s_GLFWWindowCount;

                if (s_GLFWWindowCount == 0)
                    glfwTerminate();

                return; // or throw
            }
        }

#if defined(FK_DEBUG)
        // Enable GL debug output if core 4.3+ is present
        if (GLAD_GL_VERSION_4_3) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(&GLDebugCallback, nullptr);
            // Optionally mute notifications:
            // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        }
#endif

        // Log what we actually got
        int gotMaj = 0, gotMin = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &gotMaj);
        glGetIntegerv(GL_MINOR_VERSION, &gotMin);
        const char* gl_vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        const char* gl_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        FK_CORE_INFO("OpenGL requested: {}.{} via {}", requestedMaj, requestedMin, usedEGL ? "EGL" : "GLX");
        FK_CORE_INFO("OpenGL obtained : {}.{}", gotMaj, gotMin);
        FK_CORE_INFO("GL Vendor       : {}", gl_vendor ? gl_vendor : "unknown");
        FK_CORE_INFO("GL Renderer     : {}", gl_renderer ? gl_renderer : "unknown");
        FK_CORE_INFO("GL Version str  : {}", gl_version ? gl_version : "unknown");

        // Store versions + derived GLSL string in the base Window
        SetGLInfo(gotMaj, gotMin);

#if defined(FK_HAS_STB_IMAGE)
        // Optional: set a window icon
        {
            int w = 0, h = 0, c = 0;
            if (unsigned char* img = stbi_load("Resources/Icons/icon.png", &w, &h, &c, 4)) {
                GLFWimage icon{ w, h, img };
                glfwSetWindowIcon(m_Window, 1, &icon);
                stbi_image_free(img);
            }
            else {
                FK_CORE_WARN("Failed to load window icon: Resources/Icons/icon.png");
            }
        }
#endif

        // Store our WindowData with the GLFWwindow
        glfwSetWindowUserPointer(m_Window, &m_Data);

        // NOW it's legal to set vsync
        SetVSync(true);

        // --- Callbacks ---
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.Width = static_cast<std::uint32_t>(width);
            data.Height = static_cast<std::uint32_t>(height);
            WindowResizeEvent event(data.Width, data.Height);
            if (data.EventCallback) data.EventCallback(event);
            });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent event;
            if (data.EventCallback) data.EventCallback(event);
            });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            switch (action) {
            case GLFW_PRESS: { KeyPressedEvent  e(CastToCode<KeyCode>(key), false); if (data.EventCallback) data.EventCallback(e); break; }
            case GLFW_RELEASE: { KeyReleasedEvent e(CastToCode<KeyCode>(key));        if (data.EventCallback) data.EventCallback(e); break; }
            case GLFW_REPEAT: { KeyPressedEvent  e(CastToCode<KeyCode>(key), true);  if (data.EventCallback) data.EventCallback(e); break; }
            default: break;
            }
            });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(CastToCode<KeyCode>(static_cast<int>(keycode)));
            if (data.EventCallback) data.EventCallback(event);
            });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int /*mods*/) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            switch (action) {
            case GLFW_PRESS: { MouseButtonPressedEvent  e(CastToCode<MouseCode>(button)); if (data.EventCallback) data.EventCallback(e); break; }
            case GLFW_RELEASE: { MouseButtonReleasedEvent e(CastToCode<MouseCode>(button)); if (data.EventCallback) data.EventCallback(e); break; }
            default: break;
            }
            });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            if (data.EventCallback) data.EventCallback(event);
            });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
            auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            if (data.EventCallback) data.EventCallback(event);
            });
    }

    void LinuxWindow::Shutdown() {
        FK_PROFILE_FUNCTION();

        if (!m_Window)
            return; // already shut down

        // Detach if this context is current on this thread
        if (glfwGetCurrentContext() == m_Window)
            glfwMakeContextCurrent(nullptr);

        // (optional) clear user pointer to avoid stale references in callbacks
        glfwSetWindowUserPointer(m_Window, nullptr);

        glfwDestroyWindow(m_Window);
        m_Window = nullptr;

        // Decrement defensively in all configs
        if (s_GLFWWindowCount == 0) {
            FK_CORE_CRITICAL("GLFW window count underflow in Shutdown()");
        }
        else {
            --s_GLFWWindowCount;
        }

        // if (s_GLFWWindowCount == 0)
        // glfwTerminate();
    }

    void LinuxWindow::OnUpdate() {
        FK_PROFILE_FUNCTION();

        glfwPollEvents();

        // Ensure the context is current on this thread before swapping
        if (glfwGetCurrentContext() != m_Window)
            glfwMakeContextCurrent(m_Window);

        glfwSwapBuffers(m_Window);
    }

    void LinuxWindow::SetVSync(bool enabled) {
        FK_PROFILE_FUNCTION();

        // Must have a current context on this thread
        if (glfwGetCurrentContext() != m_Window)
            glfwMakeContextCurrent(m_Window);

        glfwSwapInterval(enabled ? 1 : 0);
        m_Data.VSync = enabled;
    }

    bool LinuxWindow::IsVSync() const noexcept {
        return m_Data.VSync;
    }

} // namespace FrameKit
