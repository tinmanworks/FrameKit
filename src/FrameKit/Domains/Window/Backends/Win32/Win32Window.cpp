// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/Backends/Win32/Win32Window.cpp
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Win32 window implementation
// =============================================================================

#include "Win32Window.h"
#include "FrameKit/Utilities/Utilities.h"

#include <windowsx.h>
#include <cassert>

namespace FrameKit {

static int GetMods() {
    int m = 0;
    m |= (GetKeyState(VK_SHIFT)   & 0x8000) ? BITU(0) : 0; // Shift
    m |= (GetKeyState(VK_CONTROL) & 0x8000) ? BITU(1) : 0; // Control
    m |= (GetKeyState(VK_MENU)    & 0x8000) ? BITU(2) : 0; // Alt
    m |= (GetKeyState(VK_LWIN)    & 0x8000) ? BITU(3) : 0; // Super (L)
    m |= (GetKeyState(VK_RWIN)    & 0x8000) ? BITU(3) : 0; // Super (R)
    m |= (GetKeyState(VK_CAPITAL) & 0x0001) ? BITU(4) : 0; // Caps
    m |= (GetKeyState(VK_NUMLOCK) & 0x0001) ? BITU(5) : 0; // Num
    return m;
}
static int ScanFromLParam(LPARAM lp) { return static_cast<int>((lp >> 16) & 0xFF); }

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* self = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    switch (msg) {
    case WM_CREATE:
        if (!self) {
            auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        }
        return 0;

    case WM_CLOSE:
        if (self) {
            if (self->onCloseReq) self->onCloseReq(CloseReq{});
            self->requestClose();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        if (self) {
            self->m_w = LOWORD(lParam);
            self->m_h = HIWORD(lParam);
            if (self->onResize) self->onResize(Resize{ (int)self->m_w, (int)self->m_h });
        }
        return 0;

    case WM_DPICHANGED:
        if (self) {
            const UINT dpi = HIWORD(wParam);
            self->m_sx = self->m_sy = dpi / 96.f;
        }
        return 0;

    // Keyboard
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (self && self->onKey) {
            RawKeyEvent ev{};
            ev.key = static_cast<int>(wParam);               // If you add VK→KeyCode mapping, apply here.
            ev.scancode = ScanFromLParam(lParam);
            ev.mods = GetMods();
            ev.action = (lParam & (1<<30)) ? 2 : 1;          // 2=repeat, 1=press
            self->onKey(ev);
        }
        return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (self && self->onKey) {
            RawKeyEvent ev{};
            ev.key = static_cast<int>(wParam);
            ev.scancode = ScanFromLParam(lParam);
            ev.mods = GetMods();
            ev.action = 0;                                   // release
            self->onKey(ev);
        }
        return 0;

    // Mouse buttons
    case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
        if (self && self->onMouseBtn) {
            RawMouseBtn b{};
            b.button = (msg==WM_LBUTTONDOWN)?0:(msg==WM_RBUTTONDOWN)?1:2; // map to your MouseCode later
            b.action = 1; b.mods = GetMods();
            self->onMouseBtn(b);
        }
        return 0;

    case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP:
        if (self && self->onMouseBtn) {
            RawMouseBtn b{};
            b.button = (msg==WM_LBUTTONUP)?0:(msg==WM_RBUTTONUP)?1:2;
            b.action = 0; b.mods = GetMods();
            self->onMouseBtn(b);
        }
        return 0;

    // Mouse move and wheel
    case WM_MOUSEMOVE:
        if (self && self->onMouseMove) {
            RawMouseMove mv{ static_cast<double>(GET_X_LPARAM(lParam)),
                             static_cast<double>(GET_Y_LPARAM(lParam)) };
            self->onMouseMove(mv);
        }
        return 0;

    case WM_MOUSEWHEEL:
        if (self && self->onMouseWheel) {
            RawMouseWheel wh{}; wh.dx = 0.0; wh.dy = static_cast<short>(HIWORD(wParam)) / 120.0;
            self->onMouseWheel(wh);
        }
        return 0;

    case WM_MOUSEHWHEEL:
        if (self && self->onMouseWheel) {
            RawMouseWheel wh{}; wh.dx = static_cast<short>(HIWORD(wParam)) / 120.0; wh.dy = 0.0;
            self->onMouseWheel(wh);
        }
        return 0;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

Win32Window::Win32Window(const WindowDesc& d) {
    m_hinst = GetModuleHandleW(nullptr);

    WNDCLASSW wc{};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = m_hinst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"FrameKitWnd";
    RegisterClassW(&wc);

    std::wstring wtitle(d.title.begin(), d.title.end());
    DWORD style = d.resizable ? WS_OVERLAPPEDWINDOW : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU);
    RECT r{0,0,(LONG)d.width,(LONG)d.height};
    AdjustWindowRect(&r, style, FALSE);

    m_hwnd = CreateWindowW(
        wc.lpszClassName, wtitle.c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        nullptr, nullptr, m_hinst, this);
    assert(m_hwnd);

    // initial size and scale
    m_w = d.width; m_h = d.height;
    UINT dpi = 96;
    if (auto pGetDpiForWindow = reinterpret_cast<UINT (WINAPI*)(HWND)>(GetProcAddress(GetModuleHandleW(L"user32"), "GetDpiForWindow"))) {
        dpi = pGetDpiForWindow(m_hwnd);
    }
    m_sx = m_sy = dpi / 96.f;

    ShowWindow(m_hwnd, d.visible ? SW_SHOW : SW_HIDE);
}

Win32Window::~Win32Window() {
    if (m_hwnd) DestroyWindow(m_hwnd);
}

void Win32Window::poll() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

bool Win32Window::shouldClose() const { return m_close; }
void Win32Window::requestClose() { m_close = true; }

void Win32Window::setTitle(const std::string& t) {
    std::wstring w(t.begin(), t.end());
    SetWindowTextW(m_hwnd, w.c_str());
}

void Win32Window::setCursorMode(CursorMode m) {
    switch (m) {
    case CursorMode::Normal:
        while (ShowCursor(TRUE) < 0) {} ClipCursor(nullptr); break;
    case CursorMode::Hidden:
        while (ShowCursor(FALSE) >= 0) {} ClipCursor(nullptr); break;
    case CursorMode::Locked: {
        while (ShowCursor(FALSE) >= 0) {}
        RECT rc; GetClientRect(m_hwnd, &rc);
        POINT tl{rc.left, rc.top}, br{rc.right, rc.bottom};
        ClientToScreen(m_hwnd, &tl); ClientToScreen(m_hwnd, &br);
        RECT clip{tl.x, tl.y, br.x, br.y};
        ClipCursor(&clip);
    } break;
    }
}

static void DeleteWin32(IWindow* w) noexcept { delete w; }

static WindowPtr CreateWin32(const WindowDesc& d) {
    return WindowPtr(new Win32Window(d), &DeleteWin32);
}

// explicit registrar callable from core
extern "C" bool FrameKit_RegisterBackend_Win32() {
    return RegisterWindowBackend(WindowAPI::Win32, "Win32", &CreateWin32, 100);
}

} // namespace FrameKit

