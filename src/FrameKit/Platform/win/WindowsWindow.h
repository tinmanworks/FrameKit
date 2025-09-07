/*
 * Project: FrameKit
 * File: WindowsWindow.h
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   GLFW-based Window implementation for Windows.
 */

#pragma once

#include "FrameKit/Interface/Window.h"

#include <cstdint>
#include <string>

struct GLFWwindow; // fwd-declare to avoid pulling GLFW in the header

namespace FrameKit {

    class WindowsWindow : public Window {
    public:
        explicit WindowsWindow(const WindowProps& props);
        ~WindowsWindow() override;

        void OnUpdate() override;

        [[nodiscard]] std::uint32_t GetWidth()  const noexcept override { return m_Data.Width; }
        [[nodiscard]] std::uint32_t GetHeight() const noexcept override { return m_Data.Height; }

        // Window attributes
        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled) override;
        [[nodiscard]] bool IsVSync() const noexcept override;

        [[nodiscard]] void* GetNativeWindow() const noexcept override { return m_Window; }

        // Non-copyable / non-movable
        WindowsWindow(const WindowsWindow&) = delete;
        WindowsWindow& operator=(const WindowsWindow&) = delete;
        WindowsWindow(WindowsWindow&&) = delete;
        WindowsWindow& operator=(WindowsWindow&&) = delete;

    private:
        void Init(const WindowProps& props);
        void Shutdown();

    private:
        GLFWwindow* m_Window{ nullptr };

        struct WindowData {
            std::string       Title;
            std::uint32_t     Width{ 0 };
            std::uint32_t     Height{ 0 };
            bool              VSync{ true };
            EventCallbackFn   EventCallback;
        };

        WindowData m_Data{};
    };

} // namespace FrameKit
