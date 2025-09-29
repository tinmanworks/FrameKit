// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Addon/AddonHost.h
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-09-28
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Host for FrameKit addons.
// =============================================================================

#pragma once

#include "FrameKit/Addon/AddonBase.h"
#include "FrameKit/Addon/AddonV1.h"

#if defined(FK_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
using fk_lib_handle_t = HMODULE;
inline void* fk_get_symbol(fk_lib_handle_t lib, const char* name) { 
    return reinterpret_cast<void*>(::GetProcAddress(lib, name));
}
#else 
#include <dlfcn.h>
using fk_lib_handle_t = void*;
inline void* fk_get_symbol(fk_lib_handle_t lib, const char* name) { 
    ::dlerror(); // clear
    void* sym = ::dlsym(lib, name);
    // optional: const char* err = ::dlerror();
    return sym;
}
#endif

#include <string>
#include <stdexcept>

namespace FrameKit {
    // ---------- helpers ----------
    template<class Fn>
    inline Fn load_required(fk_lib_handle_t dll, const char* name) {
        if (auto p = reinterpret_cast<Fn>(fk_get_symbol(dll, name))) return p;
        throw std::runtime_error(std::string("missing symbol: ") + name);
    }
    template<class Fn>
    inline Fn load_optional(fk_lib_handle_t dll, const char* name) {
        return reinterpret_cast<Fn>(fk_get_symbol(dll, name));
    }

    inline bool abi_ok(AddonABIVersion have, AddonABIVersion need) {
        if (need.major == 0 && need.minor == 0) return true;
        if (have.major != need.major) return false;
        return have.minor >= need.minor; // patch ignored
    }

    using CreateAddonInstanceFn = FrameKit::IAddonBase* (FK_CDECL*)();
    using DestroyAddonInstanceFn = void (FK_CDECL*)(FrameKit::IAddonBase*);
    using GetAddonABIVersionFn = void (FK_CDECL*)(FrameKit::AddonABIVersion*);
    using GetAddonVersionTagFn = FrameKit::AddonVersionTag(FK_CDECL*)();

    struct AddonHostBaseFunctions {
        CreateAddonInstanceFn  create{};
        DestroyAddonInstanceFn destroy{};
        GetAddonABIVersionFn   get_version{};
        GetAddonVersionTagFn   get_tag{};
    };

    inline AddonHostBaseFunctions load_addonhost_base_symbols(fk_lib_handle_t sl_handle) {
        AddonHostBaseFunctions s;
        s.create = load_required<CreateAddonInstanceFn>(sl_handle, "CreateAddonInstance");
        s.destroy = load_required<DestroyAddonInstanceFn>(sl_handle, "DestroyAddonInstance");
        s.get_version = load_required<GetAddonABIVersionFn>(sl_handle, "GetAddonABIVersion");
        s.get_tag = load_required<GetAddonVersionTagFn>(sl_handle, "GetAddonVersionTag");
        return s;
    }

    // RAII for instance lifetime
    struct AddonInstance {
        IAddonBase* base_inst{};
        DestroyAddonInstanceFn destroy{};
        ~AddonInstance() { if (base_inst && destroy) destroy(base_inst); }
        AddonInstance() = default;
        AddonInstance(IAddonBase* q, DestroyAddonInstanceFn d) : base_inst(q), destroy(d) {}
        AddonInstance(AddonInstance&& o) noexcept : base_inst(o.base_inst), destroy(o.destroy) { o.base_inst = nullptr; o.destroy = nullptr; }
        AddonInstance& operator=(AddonInstance&& obj) noexcept {
            if (this != &obj) { if (base_inst && destroy) destroy(base_inst); base_inst = obj.base_inst; destroy = obj.destroy; obj.base_inst = nullptr; obj.destroy = nullptr; }
            return *this;
        }
        AddonInstance(const AddonInstance&) = delete;
        AddonInstance& operator=(const AddonInstance&) = delete;
    };

    // ------- Base AddonHost ---------
    struct IAddonHost {
        fk_lib_handle_t sl_handle{};
        AddonHostBaseFunctions ahb_fns{};
        AddonABIVersion abi{};
        AddonInstance addon_inst{};
    };

    // ---------- V1 Host ----------
    namespace AddonV1 {
        struct AddonHostV1 : IAddonHost {
            IAddonV1* instance{};
            void bind_iface() { instance = static_cast<IAddonV1*>(addon_inst.base_inst); }
            void OnUpdate() { instance->OnUpdate(); }
            void OnRender() { instance->OnRender(); }
            void OnCyclic() { instance->OnCyclic(); }
        };
    }
    // -------- traits keyed by tag --------
    template<AddonVersionTag> struct AddonTraits;

    template<> struct AddonTraits<AddonVersionTag::V1> {
        using Host = AddonV1::AddonHostV1;
        static constexpr AddonABIVersion required{ 1,0,0 };
    };

    // ---------- main creator -----------
    template<AddonVersionTag Tag> inline typename AddonTraits<Tag>::Host
        CreateAddonHost(fk_lib_handle_t sl_handle, AddonABIVersion required = AddonTraits<Tag>::required)
    {
        using T = AddonTraits<Tag>;
        typename T::Host host{};
        host.sl_handle = sl_handle;
        host.ahb_fns = load_addonhost_base_symbols(sl_handle);

        const auto reported = host.ahb_fns.get_tag();
        if (reported != Tag) throw std::runtime_error("addon tag mismatch");

        host.ahb_fns.get_version(&host.abi);
        if (!abi_ok(host.abi, required)) throw std::runtime_error("ABI incompatible");

        IAddonBase* raw = host.ahb_fns.create();
        if (!raw) throw std::runtime_error("CreateAddonInstance returned null");
        host.addon_inst = AddonInstance{ raw, host.ahb_fns.destroy };   // single construction

        host.bind_iface();
        return host;
    }
} // namespace FrameKit
