// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Interface/Backends/ImGuiAdapter.h
// Description  : ImGui bindings for FrameKit's backend-agnostic UI handle.
// =============================================================================
#pragma once
#include <imgui.h>
#include <string_view>
#include "FrameKit/Interface/GlobalContext.h"

namespace FrameKit::ImGuiBackend {

// Unique tag object whose address identifies the ImGui backend.
inline constexpr char kTagObject = 0;
inline constexpr const void* kTag = &kTagObject;

// Factory: wrap an ImGuiContext* as a InterfaceContextHandle.
FK_NODISCARD inline InterfaceContextHandle MakeHandle(ImGuiContext* ctx) noexcept {
    return InterfaceContextHandle{kTag, ctx};
}

// RAII: set ImGui context on construction, restore previous on destruction.
class ContextScope {
public:
    explicit ContextScope(InterfaceContextHandle h) noexcept
        : prev_(ImGui::GetCurrentContext()) {
        if (h.tag == kTag) ImGui::SetCurrentContext(static_cast<ImGuiContext*>(h.ptr));
    }
    explicit ContextScope(const GlobalContext& gc) noexcept : ContextScope(gc.ui) {}

    ContextScope(const ContextScope&) = delete;
    ContextScope& operator=(const ContextScope&) = delete;

    ~ContextScope() { ImGui::SetCurrentContext(prev_); }

private:
    ImGuiContext* prev_ = nullptr;
};

// RAII: open an ImGui window; closes on destruction.
class WindowScope {
public:
    WindowScope(const GlobalContext& gc, std::string_view title, ImGuiWindowFlags flags = 0)
        : ctx_(gc), open_(true) { ImGui::Begin(title.data(), &open_, flags); }

    WindowScope(UIContextHandle h, std::string_view title, ImGuiWindowFlags flags = 0)
        : ctx_(h), open_(true) { ImGui::Begin(title.data(), &open_, flags); }

    WindowScope(const WindowScope&) = delete;
    WindowScope& operator=(const WindowScope&) = delete;

    ~WindowScope() { ImGui::End(); }

private:
    ContextScope ctx_;
    bool open_ = true;
};

// Convenience macros (scoped usage).
#ifndef FK_JOIN
  #define FK_JOIN_AGAIN(a,b) a##b
  #define FK_JOIN(a,b) FK_JOIN_AGAIN(a,b)
#endif

#define FK_IMGUI_WINDOW(ctx_or_gc, title) \
    ::FrameKit::ImGuiBackend::WindowScope FK_JOIN(_fk_imgui_window_, __COUNTER__){ (ctx_or_gc), (title) }

#define FK_IMGUI_SET_CONTEXT(ctx_or_gc) \
    ::FrameKit::ImGuiBackend::ContextScope FK_JOIN(_fk_imgui_ctx_, __COUNTER__){ (ctx_or_gc) }

} // namespace FrameKit::ImGuiBackend
