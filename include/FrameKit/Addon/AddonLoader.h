// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/AddonLoader.h
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Runtime loader for addons using the FrameKit C-ABI.
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"
#include "FrameKit/Addon/FKABI.h"
#include "FrameKit/Addon/FKHostV1.h"
#include "FrameKit/Addon/FKAddonV1.h"

#include <filesystem>
#include <string>
#include <optional>

#if defined(FK_PLATFORM_WINDOWS)
#include <windows.h>
using fk_lib_handle_t = HMODULE;
inline void* fk_get_symbol(fk_lib_handle_t h, const char* n) { return (void*)GetProcAddress(h, n); }
#else
#include <dlfcn.h>
using fk_lib_handle_t = void*;
inline void* fk_get_symbol(fk_lib_handle_t h, const char* n) { dlerror(); return dlsym(h, n); }
#endif

namespace FrameKit {

    using GetInterfaceFn = void* (FK_CDECL*)(const char*, uint32_t) noexcept;

    struct LoadedAddon {
        std::filesystem::path path;          // canonical load path
        fk_lib_handle_t       handle{};      // OS library handle
        FK_AddonInfo          info{};
        GetInterfaceFn        addon_get{};   // addon's GetInterface
        void                (*addon_shutdown)() noexcept {}; // optional ShutdownAddon()
        const FK_AddonV1*     addon_v1{};    // lifecycle iface
    };

    struct IHostGetProvider {
        virtual ~IHostGetProvider() = default;
        virtual void* HostGet(const char* id, uint32_t min_ver) noexcept = 0; // aggregates FK + app tables
    };

    class AddonLoader {
    public:
        explicit AddonLoader(IHostGetProvider& provider);
        std::optional<LoadedAddon> Load(const std::filesystem::path& lib);
        void Unload(LoadedAddon& a) noexcept;

    private:
        IHostGetProvider& host_provider_;
        static fk_lib_handle_t open_library(const std::filesystem::path& p);
        static void close_library(fk_lib_handle_t h) noexcept;
    };

} // namespace FrameKit
