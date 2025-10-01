// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Addon/AddonLoader.cpp
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-01
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        
// =============================================================================


#include "FrameKit/Addon/AddonLoader.h"

#include <stdexcept>

#if defined(FK_PLATFORM_WINDOWS)
static std::string last_err() { return "win32"; }
#else
static std::string last_err() { const char* e = dlerror(); return e ? e : "dlerror-null"; }
#endif

namespace FrameKit {
    fk_lib_handle_t AddonLoader::open_library(const std::filesystem::path& p) {
#if defined(FK_PLATFORM_WINDOWS)
        HMODULE h = LoadLibraryW(p.wstring().c_str());
        if (!h) throw std::runtime_error("LoadLibraryW failed");
        return h;
#else
        void* h = dlopen(p.string().c_str(), RTLD_NOW);
        if (!h) throw std::runtime_error(std::string("dlopen failed: ") + last_err());
        return h;
#endif
    }

    void AddonLoader::close_library(fk_lib_handle_t h) noexcept {
#if defined(FK_PLATFORM_WINDOWS)
        if (h) FreeLibrary(h);
#else
        if (h) dlclose(h);
#endif
    }

    static IHostGetProvider* g_host_provider = nullptr;
    static void* FK_CDECL HostGetBridge(const char* id, uint32_t min_ver) noexcept {
        return g_host_provider ? g_host_provider->HostGet(id, min_ver) : nullptr;
    }

    AddonLoader::AddonLoader(IHostGetProvider& provider) : host_provider_(provider) {
        g_host_provider = &host_provider_;
    }

    std::optional<LoadedAddon> AddonLoader::Load(const std::filesystem::path& lib) {
        fk_lib_handle_t h{};
        try { h = open_library(lib); }
        catch (...) { return std::nullopt; }

        auto get_info = reinterpret_cast<void (FK_CDECL*)(FK_AddonInfo*)>(fk_get_symbol(h, "GetAddonInfo"));
        auto set_host = reinterpret_cast<void (FK_CDECL*)(FK_GetInterfaceFn)>(fk_get_symbol(h, "SetHostGetter"));
        auto get_iface = reinterpret_cast<void* (FK_CDECL*)(const char*, uint32_t)>(fk_get_symbol(h, "GetInterface"));
        auto shut_fn = reinterpret_cast<void (FK_CDECL*)(void) noexcept>(fk_get_symbol(h, "ShutdownAddon"));

        if (!get_info || !set_host || !get_iface || !shut_fn) { close_library(h); return std::nullopt; }

        FK_AddonInfo info{}; get_info(&info);
        if (info.abi_major != 1) { close_library(h); return std::nullopt; }

        // provide host getter
        auto bridge = +[](const char* id, uint32_t ver) noexcept -> void* {
            // will be replaced per-instance using TLS or capture trampoline
            return nullptr;
            };

        set_host(&HostGetBridge);

        // pull addon lifecycle
        auto* a1 = (const FK_AddonV1*)get_iface(FK_IFACE_ADDON_V1, 1);
        if (!a1 || a1->size < sizeof(FK_AddonV1) || a1->version < 1) { close_library(h); return std::nullopt; }

        try { if (a1->Initialize) a1->Initialize(); }
        catch (...) { close_library(h); return std::nullopt; }

        LoadedAddon out{};
        out.handle = h;
        out.info = info;
        out.addon_get = reinterpret_cast<FK_GetInterfaceFn>(get_iface);
        out.addon_shutdown = shut_fn;
        out.addon_v1 = a1;
        return out;
    }

    void AddonLoader::Unload(LoadedAddon& a) noexcept {
        if (a.addon_v1 && a.addon_v1->Shutdown) { try { a.addon_v1->Shutdown(); } catch (...) {} }
        if (a.addon_shutdown) { try { a.addon_shutdown(); } catch (...) {} }
        close_library(a.handle);
        a = {};
    }

} // namespace FrameKit