/*
 * Project: FrameKit
 * File: Window.cpp
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   Factory that creates the platform window implementation
 *   + helper to store OpenGL version and matching GLSL #version.
 */

#include "FrameKit/Interface/Window.h"
#include "FrameKit/Base/Assert.h"

#if defined(FK_PLATFORM_WINDOWS)
    #include "FrameKit/Platform/win/WindowsWindow.h"
#elif defined(FK_PLATFORM_LINUX)
    #include "FrameKit/Platform/linux/LinuxWindow.h"
#endif

#include <string>

namespace { // Desktop GL (Windows/Linux) GLSL mapper
    inline std::string GLSLVersionFromGL(int maj, int min) {
        const int enc = maj * 100 + min * 10; // 4.6 -> 460, 3.3 -> 330
        if (enc >= 460) return "#version 460 core";
        else if (enc >= 450) return "#version 450 core";
        else if (enc >= 440) return "#version 440 core";
        else if (enc >= 430) return "#version 430 core";
        else if (enc >= 420) return "#version 420 core";
        else if (enc >= 410) return "#version 410 core";
        else if (enc >= 400) return "#version 400 core";
        else if (enc >= 330) return "#version 330 core";
        else if (enc >= 320) return "#version 150"; // GL 3.2 baseline (mac-style)
        else if (enc >= 310) return "#version 140";
        else if (enc >= 300) return "#version 130";
        else                 return "#version 120";
    }
}

namespace FrameKit {

    Scope<Window> Window::Create(const WindowProps& props) {
#if defined(FK_PLATFORM_WINDOWS)
        return CreateScope<WindowsWindow>(props);
#elif defined(FK_PLATFORM_LINUX)
        return CreateScope<LinuxWindow>(props);
#else
        FK_CORE_ASSERT(false, "Unknown or unsupported platform!");
        return nullptr;
#endif
        }

    // Called by platform windows after they query GL_MAJOR_VERSION / GL_MINOR_VERSION
    void Window::SetGLInfo(int maj, int min) noexcept {
        m_GLVersionMajor = maj;
        m_GLVersionMinor = min;
        m_GlslVersion = GLSLVersionFromGL(maj, min);
    }

} // namespace FrameKit
