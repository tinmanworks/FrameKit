// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Window/WindowEventBridge.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Binds window events to global event handler
// =============================================================================

#pragma once

#include "FrameKit/Window/IWindow.h"
#include "FrameKit/Events/WindowEvent.h"
#include "FrameKit/Events/KeyEvent.h"
#include "FrameKit/Events/MouseEvent.h"
#include "FrameKit/Events/GlobalEventHandler.h"
#include "FrameKit/Input/KeyCodes.h"
#include "FrameKit/Input/MouseCodes.h"

namespace FrameKit {

inline KeyCode   ToKeyCodeFromRaw(int raw)   { return static_cast<KeyCode>(raw); }   // GLFW path: identical
inline MouseCode ToMouseCodeFromRaw(int raw) { return static_cast<MouseCode>(raw); } // GLFW path: identical

inline void BindWindowToGlobalEvents(IWindow& w) {
    w.onCloseReq = [](const CloseReq&) {
        WindowCloseEvent e; GlobalEventHandler::Get().Emit(e);
    };
    w.onResize = [](const Resize& r) {
        WindowResizeEvent e(static_cast<uint32_t>(r.width), static_cast<uint32_t>(r.height));
        GlobalEventHandler::Get().Emit(e);
    };
    w.onKey = [](const RawKeyEvent& k) {
        KeyCode kc = ToKeyCodeFromRaw(k.key);
        if (k.action == 1) {
            KeyPressedEvent e(kc, k.scancode, k.mods, false); GlobalEventHandler::Get().Emit(e);
        } else if (k.action == 2) {
            KeyPressedEvent e(kc, k.scancode, k.mods, true);  GlobalEventHandler::Get().Emit(e);
        } else {
            KeyReleasedEvent e(kc, k.scancode, k.mods);       GlobalEventHandler::Get().Emit(e);
        }
    };
    w.onMouseBtn = [](const RawMouseBtn& b) {
        MouseCode mb = ToMouseCodeFromRaw(b.button);
        if (b.action) { MouseButtonPressedEvent e(mb);  GlobalEventHandler::Get().Emit(e); }
        else          { MouseButtonReleasedEvent e(mb); GlobalEventHandler::Get().Emit(e); }
    };
    w.onMouseMove = [](const RawMouseMove& m) {
        MouseMovedEvent e(static_cast<float>(m.x), static_cast<float>(m.y)); GlobalEventHandler::Get().Emit(e);
    };
    w.onMouseWheel = [](const RawMouseWheel& v) {
        MouseScrolledEvent e(static_cast<float>(v.dx), static_cast<float>(v.dy)); GlobalEventHandler::Get().Emit(e);
    };
}

} // namespace FrameKit
