/*
 * Project: FrameKit
 * File: Window.h
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   Abstract desktop window interface and creation helper.
 */

#pragma once

#include "FrameKit/Base/Base.h"
#include "FrameKit/Events/Event.h"

#include <cstdint>
#include <functional>
#include <string>

namespace FrameKit {

    struct WindowProps {
        std::string   Title{ "FrameKit Engine" };
        std::uint32_t Width{ 1600 };
        std::uint32_t Height{ 900 };

        WindowProps() = default;
        explicit WindowProps(std::string title) noexcept
            : Title(std::move(title)) {
        }
        WindowProps(std::string title, std::uint32_t width, std::uint32_t height) noexcept
            : Title(std::move(title)), Width(width), Height(height) {
        }
    };

    // Interface representing a desktop system window
    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        [[nodiscard]] virtual std::uint32_t GetWidth()  const noexcept = 0;
        [[nodiscard]] virtual std::uint32_t GetHeight() const noexcept = 0;

        // Window attributes
        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enabled) = 0;
        [[nodiscard]] virtual bool IsVSync() const noexcept = 0;

        // Native backend window pointer (e.g., GLFWwindow*, HWND, etc.)
        [[nodiscard]] virtual void* GetNativeWindow() const noexcept = 0;

        // OpenGL/GLSL info (set by platform implementation after context creation)
        [[nodiscard]] int  GetGLMajor() const noexcept { return m_GLVersionMajor; }
        [[nodiscard]] int  GetGLMinor() const noexcept { return m_GLVersionMinor; }
        [[nodiscard]] const std::string& GetGlslVersion() const noexcept { return m_GlslVersion; }

        static Scope<Window> Create(const WindowProps& props = WindowProps());

    protected:
        // Platform window should call this once after creating the GL context
        // and querying GL_MAJOR_VERSION/GL_MINOR_VERSION. The base will derive
        // a matching GLSL "#version ..." string.
        void SetGLInfo(int maj, int min) noexcept;

    private:
        int         m_GLVersionMajor = 0;
        int         m_GLVersionMinor = 0;
        std::string m_GlslVersion;        // e.g., "#version 330 core"
    };

} // namespace FrameKit
