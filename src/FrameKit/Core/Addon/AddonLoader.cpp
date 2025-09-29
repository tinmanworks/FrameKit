#include "FrameKit/Addon/AddonLoader.h"

namespace FrameKit {
    fk_lib_handle_t AddonLoader::open_library(const std::filesystem::path& p) {
#if defined(FK_PLATFORM_WINDOWS)
    HMODULE h = ::LoadLibraryW(p.wstring().c_str());
    if (!h) throw std::runtime_error("LoadLibraryW failed");
    return h;
#else
    // dlopen needs char*; prefer absolute path to avoid rpath surprises
    void* h = ::dlopen(p.string().c_str(), RTLD_NOW);
    if (!h) {
        const char* err = ::dlerror();
        throw std::runtime_error(err ? err : "dlopen failed");
    }
    return h;
#endif

    }

    void AddonLoader::close_library(fk_lib_handle_t h) noexcept {
#if defined(FK_PLATFORM_WINDOWS)
        if (h) ::FreeLibrary(h);
#else
        if (h) ::dlclose(h);
#endif
    }

    IAddonHost* AddonLoader::LoadAddon(const std::filesystem::path& p) {
        fk_lib_handle_t lib = open_library(p);
        auto get_tag_fn = reinterpret_cast<GetAddonVersionTagFn>(fk_get_symbol(lib, "GetAddonVersionTag"));
        if (!get_tag_fn) { 
            close_library(lib); 
            printf("Missing symbol: GetAddonVersionTag\n"); 
            throw std::runtime_error("missing symbol: GetAddonVersionTag");
        }
        AddonVersionTag tag = get_tag_fn();

        switch (tag) {
        case AddonVersionTag::V1: {
            auto h = CreateAddonHost<AddonVersionTag::V1>(lib);
            return new AddonV1::AddonHostV1(std::move(h));
        }
        default:
            close_library(lib);
            throw std::runtime_error("unknown AddonVersionTag");
        }
    }


    bool AddonLoader::UnloadAddon(IAddonHost& addonHost) {
        // destroy addon instance via DLL function
        if (addonHost.addon_inst.base_inst && addonHost.ahb_fns.destroy) {
            addonHost.ahb_fns.destroy(addonHost.addon_inst.base_inst);
            addonHost.addon_inst.base_inst = nullptr;
        }
        // then unload the library
        close_library(addonHost.sl_handle);
        // finally delete the host object
        delete& addonHost;
        return true;
    }
} // namespace FrameKit