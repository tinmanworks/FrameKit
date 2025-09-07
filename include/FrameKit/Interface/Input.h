/*
 * Project: FrameKit
 * File: Input.h
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   Platform-agnostic input queries for keyboard and mouse,
 *   no GLM dependency.
 */

#pragma once

#include "FrameKit/Interface/KeyCodes.h"
#include "FrameKit/Interface/MouseCodes.h"
// TODO: Look into RadarScape for input handling details

namespace FrameKit {

    struct MousePos {
        float x;
        float y;
    };

    class Input {
    public:
        // Keyboard
        [[nodiscard]] static bool IsKeyPressed(KeyCode key) noexcept;

        // Mouse buttons
        [[nodiscard]] static bool IsMouseButtonPressed(MouseCode button) noexcept;

        // Mouse position
        [[nodiscard]] static MousePos GetMousePosition() noexcept;

        [[nodiscard]] static float GetMouseX() noexcept {
            return GetMousePosition().x;
        }

        [[nodiscard]] static float GetMouseY() noexcept {
            return GetMousePosition().y;
        }
    };

} // namespace FrameKit
