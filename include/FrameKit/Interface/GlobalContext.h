/*
 * Project   : FrameKit
 * File      : GlobalContext.h
 * Author    : George Gil
 * Date      : 2025-08-12
 * License   : MIT
 * Description:
 *   Small utilities for working with Dear ImGui:
 *   - GlobalContext: holds pointers to the active ImGui context and main window.
 *   - ImGuiContextScope: RAII that sets an ImGui context and restores the previous one.
 *   - ImGuiWindowScope: RAII that sets context and opens/closes an ImGui window.
 *   - Convenience macros to create scoped windows without manual variable names.
 */

 // TODO : Make Global Context generalised and configurable for each unique application setup
 // TODO : Maybe rename Global Context to something more specific to interface if that makes sense
#pragma once

#include <string_view>

namespace FrameKit {

// Minimal container for cross-system state used by UI layers.
struct GlobalContext {
    ImGuiContext* imguiCtx = nullptr;
    void*         mainWindow = nullptr;

    constexpr GlobalContext() noexcept = default;
    explicit constexpr GlobalContext(ImGuiContext* ctx) noexcept : imguiCtx(ctx) {}

    [[nodiscard]] constexpr bool valid() const noexcept { return imguiCtx != nullptr; }
};

// RAII: set ImGui context on construction, restore previous on destruction.
class ImGuiContextScope {
public:
    explicit ImGuiContextScope(ImGuiContext* ctx) noexcept
        : prev_(ImGui::GetCurrentContext())
    {
        if (ctx) ImGui::SetCurrentContext(ctx);
    }

    explicit ImGuiContextScope(const GlobalContext& gc) noexcept
        : ImGuiContextScope(gc.imguiCtx) {}

    ImGuiContextScope(const ImGuiContextScope&) = delete;
    ImGuiContextScope& operator=(const ImGuiContextScope&) = delete;
    ImGuiContextScope(ImGuiContextScope&&) = delete;
    ImGuiContextScope& operator=(ImGuiContextScope&&) = delete;

    ~ImGuiContextScope() {
        ImGui::SetCurrentContext(prev_);
    }

private:
    ImGuiContext* prev_ = nullptr;
};

// RAII: activate context and open an ImGui window; closes the window on destruction.
class ImGuiWindowScope {
public:
    ImGuiWindowScope(const GlobalContext& gc,
                     std::string_view title,
                     ImGuiWindowFlags flags = 0)
        : ctxScope_(gc), open_(true)
    {
        ImGui::Begin(title.data(), &open_, flags);
    }

    ImGuiWindowScope(ImGuiContext* ctx,
                     std::string_view title,
                     ImGuiWindowFlags flags = 0)
        : ctxScope_(ctx), open_(true)
    {
        ImGui::Begin(title.data(), &open_, flags);
    }

    ImGuiWindowScope(const ImGuiWindowScope&) = delete;
    ImGuiWindowScope& operator=(const ImGuiWindowScope&) = delete;
    ImGuiWindowScope(ImGuiWindowScope&&) = delete;
    ImGuiWindowScope& operator=(ImGuiWindowScope&&) = delete;

    ~ImGuiWindowScope() {
        ImGui::End();
    }

private:
    ImGuiContextScope ctxScope_;
    bool open_ = true;
};

} // namespace FrameKit

// ---------- Convenience macros (optional) ----------
#ifndef FK_JOIN
  #define FK_JOIN_AGAIN(a,b) a##b
  #define FK_JOIN(a,b) FK_JOIN_AGAIN(a,b)
#endif

// Open a scoped ImGui window with automatic End() and context restore.
#define FK_IMGUI_WINDOW(ctx_or_gc, title) \
    ::FrameKit::ImGuiWindowScope FK_JOIN(_fk_imgui_window_, __COUNTER__){ (ctx_or_gc), (title) }

// Set an ImGui context for the current scope (no window).
#define FK_IMGUI_SET_CONTEXT(ctx_or_gc) \
    ::FrameKit::ImGuiContextScope FK_JOIN(_fk_imgui_ctx_, __COUNTER__){ (ctx_or_gc) }
