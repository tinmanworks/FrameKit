// =============================================================================
// Project      : FlightDeck
// File         : Addons/DemoAddon/Addon_Demo.cpp
// Author       : George Gil
// Created      : 2025-10-08
// Updated      : 2025-10-08
// Description  :
//        Demo addon implementing FrameKit.Addon.V1
// =============================================================================

#define FRAMEKIT_BUILDING_ADDON
#include "FrameKit/Engine/Defines.h"
#include "FrameKit/Addon/FKABI.h"
#include "FrameKit/Addon/FKHostV1.h"
#include "FrameKit/Addon/FKAddonV1.h"
#include <atomic>
#include <string.h>
//#include "imgui.h"
#include <iostream>


static FK_GetInterfaceCtxFn g_host_get = nullptr;
static void* g_ctx = nullptr;
static const FK_HostV1* g_host = nullptr;
static std::atomic<bool>    g_inited{ false };

// lifecycle
static void A_Init() noexcept {
    g_host = (const FK_HostV1*)(g_host_get ? g_host_get(g_ctx, FK_IFACE_HOST_V1, 1) : nullptr);
    if (g_host && g_host->Log) g_host->Log(0, "Addon_Demo: Initialize");
    g_inited = true;
}

static void A_Update() noexcept {
    if (g_host && g_host->Log) g_host->Log(0, "Addon_Demo: Update");
}

static void A_Render() noexcept { /* no-op */ }

static void A_Cyclic() noexcept { /* no-op */ }

static void A_Shutdown() noexcept {
    if (g_host && g_host->Log) g_host->Log(0, "Addon_Demo: Shutdown");
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
    o->name = "Addon_Demo";
}
FK_API void FK_CDECL SetHostGetterEx(FK_GetInterfaceCtxFn fn, void* ctx) noexcept {
    g_host_get = fn; g_ctx = ctx;
}
FK_API void* FK_CDECL GetInterface(const char* id, uint32_t min_ver) noexcept {
    if (!id) return nullptr;
    if (strcmp(id, FK_IFACE_ADDON_V1) == 0 && min_ver <= 1) return (void*)&g_fk_addon;
    return nullptr;
}
FK_API void FK_CDECL ShutdownAddon() noexcept { /* final cleanup if needed */ }