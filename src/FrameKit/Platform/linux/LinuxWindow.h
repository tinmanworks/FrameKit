/*
 * Project: FrameKit
 * File: LinuxWindow.h
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   GLFW-based Window implementation for Linux.
 */

#pragma once

#include "FrameKit/Interface/Window.h"

#include <cstdint>
#include <string>

 // Forward declare to avoid including GLFW in the header
struct GLFWwindow;

namespace FrameKit {

    class LinuxWindow : public Window {
    public:
        explicit LinuxWindow(const WindowProps& props);
        ~LinuxWindow() override;

        // Per-frame update (poll events + swap buffers)
        void OnUpdate() override;

        // Dimensions
        [[nodiscard]] std::uint32_t GetWidth()  const noexcept override { return m_Data.Width; }
        [[nodiscard]] std::uint32_t GetHeight() const noexcept override { return m_Data.Height; }

        // Window attributes
        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled) override;
        [[nodiscard]] bool IsVSync() const noexcept override;

        // Native backend pointer (GLFWwindow*)
        [[nodiscard]] void* GetNativeWindow() const noexcept override { return m_Window; }

        // Non-copyable / non-movable
        LinuxWindow(const LinuxWindow&) = delete;
        LinuxWindow& operator=(const LinuxWindow&) = delete;
        LinuxWindow(LinuxWindow&&) = delete;
        LinuxWindow& operator=(LinuxWindow&&) = delete;

    private:
        void Init(const WindowProps& props);
        void Shutdown();

    private:
        GLFWwindow* m_Window{ nullptr };

        struct WindowData {
            std::string     Title;
            std::uint32_t   Width{ 0 };
            std::uint32_t   Height{ 0 };
            bool            VSync{ true };
            EventCallbackFn EventCallback;
        };

        WindowData m_Data{};
    };

} // namespace FrameKit
