// =============================================================================
// Project      : FrameKit
// File         : Addons/HelloAddon/src/HelloAddon.cpp
// Author       : George Gil
// Created      : 2025-10-08
// Updated      : 2025-10-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Demo addon using FK host and Sandbox ImGui host
// =============================================================================
#ifndef FRAMEKIT_BUILDING_ADDON
#define FRAMEKIT_BUILDING_ADDON
#endif

#include "FrameKit/Engine/Defines.h"
#include "FrameKit/Addon/FKABI.h"
#include "FrameKit/Addon/FKHostV1.h"
#include "FrameKit/Addon/FKAddonV1.h"
#include "FDAExt.h"

#include <atomic>
#include <string.h>
#include <imgui.h>      // the addon can include dear imgui too

static FK_GetInterfaceCtxFn g_host_get = nullptr;
static void* g_ctx = nullptr;
static const FK_HostV1* g_fk = nullptr;
static const SB_ImGuiHostV1* g_imgui = nullptr;
static std::atomic<bool>    g_inited{ false };

static void A_Init() noexcept {
    g_fk = (const FK_HostV1*)g_host_get(g_ctx, FK_IFACE_HOST_V1, 1);
    g_imgui = (const SB_ImGuiHostV1*)g_host_get(g_ctx, SB_IFACE_IMGUI_HOST_V1, 1);
    if (g_fk && g_fk->Log) g_fk->Log(0, "HelloAddon: Initialize");
    g_inited = true;
}

static void A_Update() noexcept {
    if (g_fk && g_fk->Log) g_fk->Log(0, "HelloAddon: Update");
}

static void A_Render() noexcept {
    // Bind the app's ImGui context and draw
    if (!g_imgui || !g_imgui->GetImGuiContext) return;
    if (void* ctx = g_imgui->GetImGuiContext()) {
        ImGuiContext* prev = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext((ImGuiContext*)ctx);
        if (ImGui::Begin("HelloAddon Panel")) {
            ImGui::TextUnformatted("Hello from addon UI");
        }
        ImGui::End();
        ImGui::SetCurrentContext(prev);
    }
}

static void A_Cyclic() noexcept {}
static void A_Shutdown() noexcept {
    if (g_fk && g_fk->Log) g_fk->Log(0, "HelloAddon: Shutdown");
    g_inited = false;
}

static const FK_AddonV1 g_fk_addon{
    1u, sizeof(FK_AddonV1),
    &A_Init, &A_Update, &A_Render, &A_Cyclic, &A_Shutdown
};

// exports
FK_API void FK_CDECL GetAddonInfo(FK_AddonInfo* o) noexcept {
    if (!o) return;
    o->abi_major = 1; o->abi_minor = 0; o->abi_patch = 0;
    o->name = "HelloAddon";
}
FK_API void FK_CDECL SetHostGetterEx(FK_GetInterfaceCtxFn fn, void* ctx) noexcept { g_host_get = fn; g_ctx = ctx; }
FK_API void* FK_CDECL GetInterface(const char* id, uint32_t min_ver) noexcept {
    if (id && strcmp(id, FK_IFACE_ADDON_V1) == 0 && min_ver <= 1) return (void*)&g_fk_addon;
    // optionally also expose SB_IFACE_IMGUI_PANEL_V1 for host-driven UI
    return nullptr;
}
FK_API void FK_CDECL ShutdownAddon() noexcept {}
