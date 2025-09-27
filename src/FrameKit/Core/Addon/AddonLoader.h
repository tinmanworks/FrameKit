#pragma once

#include "FrameKit/Core/Addon/AddonHost.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <stdexcept>

namespace FrameKit {

    using GetAddonVersionTagFn = FrameKit::AddonVersionTag (FK_CDECL*)();

    class AddonLoader {
    public:
        AddonLoader() = default;
        ~AddonLoader() = default;

        IAddonHost* LoadAddon(const std::filesystem::path& p);
        bool UnloadAddon(IAddonHost& addonHost);

    private:
        static fk_lib_handle_t open_library(const std::filesystem::path& p);
        static void close_library(fk_lib_handle_t h) noexcept;
    };

} // namespace FrameKit