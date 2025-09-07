#include "FrameKit/Interface/Input.h"

// TODO: Look into RadarScape for input handling details

namespace FrameKit {

    bool Input::IsKeyPressed(KeyCode /*key*/) noexcept {
        return false;
    }

    bool Input::IsMouseButtonPressed(MouseCode /*button*/) noexcept {
        return false;
    }

    MousePos Input::GetMousePosition() noexcept {
        return { 0.0f, 0.0f };
    }

} // namespace FrameKit
