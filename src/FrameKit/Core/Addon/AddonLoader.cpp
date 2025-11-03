// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Addon/AddonLoader.cpp
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Loader implementation. Uses SetHostGetterEx with per-instance context.
// =============================================================================

#include "FrameKit/Addon/AddonLoader.h"
#include <stdexcept>
#include <string>

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

    // Context bridge for SetHostGetterEx
    static void* FK_CDECL HostGetCtx(void* ctx, const char* id, uint32_t min_ver) noexcept {
        auto* prov = static_cast<IHostGetProvider*>(ctx);
        return prov ? prov->HostGet(id, min_ver) : nullptr;
    }

    AddonLoader::AddonLoader(IHostGetProvider& provider) : host_provider_(provider) {}

    std::optional<LoadedAddon> AddonLoader::Load(const std::filesystem::path& lib) {
        fk_lib_handle_t h{};
        try { h = open_library(lib); }
        catch (...) { return std::nullopt; }

        auto get_info = reinterpret_cast<void (FK_CDECL*)(FK_AddonInfo*) noexcept>(fk_get_symbol(h, "GetAddonInfo"));
        auto set_host = reinterpret_cast<void (FK_CDECL*)(FK_GetInterfaceCtxFn, void*) noexcept>(fk_get_symbol(h, "SetHostGetterEx"));
        auto get_iface = reinterpret_cast<void* (FK_CDECL*)(const char*, uint32_t) noexcept>(fk_get_symbol(h, "GetInterface"));
        auto shut_fn = reinterpret_cast<void (FK_CDECL*)(void) noexcept>(fk_get_symbol(h, "ShutdownAddon"));

        if (!get_info || !set_host || !get_iface || !shut_fn) { close_library(h); return std::nullopt; }

        FK_AddonInfo info{}; get_info(&info);
        if (info.abi_major != 1) { close_library(h); return std::nullopt; }

        // Bind host getter with this manager as context
        set_host(&HostGetCtx, &host_provider_);

        // Pull lifecycle iface
        auto* a1 = static_cast<const FK_AddonV1*>(get_iface(FK_IFACE_ADDON_V1, 1));
        if (!a1 || a1->size < sizeof(FK_AddonV1) || a1->version < 1) { close_library(h); return std::nullopt; }

        try { if (a1->Initialize) a1->Initialize(); }
        catch (...) { close_library(h); return std::nullopt; }

        LoadedAddon out{};
        // Canonicalize the path for stable identity
        std::error_code ec;
        out.path   = std::filesystem::weakly_canonical(lib, ec);
        if (ec) out.path = lib;
        out.handle = h;
        out.info = info;
        out.addon_get = get_iface;
        out.addon_shutdown = shut_fn;
        out.addon_v1 = a1;
        return out;
    }

    void AddonLoader::Unload(LoadedAddon& a) noexcept {
        try { if (a.addon_v1 && a.addon_v1->Shutdown) a.addon_v1->Shutdown(); } catch (...) {}
        try { if (a.addon_shutdown) a.addon_shutdown(); } catch (...) {}
        close_library(a.handle);
        a = {}; // poison
    }

} // namespace FrameKit
