// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Interface/Input.h
// Author       : George Gil
// Created      : 2025-08-11
// Updated      : 2025-09-07
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Platform-agnostic input queries for keyboard and mouse,
// =============================================================================

#pragma once

#include "FrameKit/Base/Base.h"
#include "FrameKit/Interface/KeyCodes.h"
#include "FrameKit/Interface/MouseCodes.h"

namespace FrameKit {

    struct MousePos {
        float x;
        float y;
    };

    class Input {
    public:
        // Keyboard
        FK_NODISCARD static bool IsKeyPressed(KeyCode key) noexcept;

        // Mouse buttons
        FK_NODISCARD static bool IsMouseButtonPressed(MouseCode button) noexcept;

        // Mouse position
        FK_NODISCARD static MousePos GetMousePosition() noexcept;

        FK_NODISCARD static float GetMouseX() noexcept {
            return GetMousePosition().x;
        }

        FK_NODISCARD static float GetMouseY() noexcept {
            return GetMousePosition().y;
        }
    };

} // namespace FrameKit
