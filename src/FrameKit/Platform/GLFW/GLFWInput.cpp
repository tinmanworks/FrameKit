/*
 * Project: FrameKit
 * File: WindowsInput.cpp
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 */

#include "FrameKit/Interface/Input.h"
#include "FrameKit/Application/Application.h"
#include "FrameKit/Interface/Window.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <type_traits>

namespace FrameKit {

    template <typename T>
    static inline int ToGLFW(T code) noexcept {
        if constexpr (std::is_enum_v<T>) {
            using U = std::underlying_type_t<T>;
            return static_cast<int>(static_cast<U>(code));
        }
        else {
            return static_cast<int>(code);
        }
    }

    static inline GLFWwindow* GetGLFWWnd() noexcept {
        return static_cast<GLFWwindow*>(ApplicationUI::Get().GetWindow().GetNativeWindow());
    }

    bool Input::IsKeyPressed(KeyCode key) noexcept {
        if (auto* wnd = GetGLFWWnd()) {
            const int state = glfwGetKey(wnd, ToGLFW(key));
            return state == GLFW_PRESS || state == GLFW_REPEAT;
        }
        return false;
    }

    bool Input::IsMouseButtonPressed(MouseCode button) noexcept {
        if (auto* wnd = GetGLFWWnd()) {
            const int state = glfwGetMouseButton(wnd, ToGLFW(button));
            return state == GLFW_PRESS;
        }
        return false;
    }

    MousePos Input::GetMousePosition() noexcept {
        if (auto* wnd = GetGLFWWnd()) {
            double x = 0.0, y = 0.0;
            glfwGetCursorPos(wnd, &x, &y);
            return { static_cast<float>(x), static_cast<float>(y) };
        }
        return { 0.0f, 0.0f };
    }


} // namespace FrameKit
