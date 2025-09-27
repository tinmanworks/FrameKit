#pragma once

#include "FrameKit/Core/Addon/AddonLoader.h"

#include <filesystem>
#include <vector>
#include <memory>

namespace FrameKit {
    struct LoadedAddon {
        int Id;
        std::filesystem::path path;
        IAddonHost* host{};
        AddonVersionTag tag{};
        AddonABIVersion abi{};
    };

    class AddonManager {

    public:
        void SetAddonDirectory(const std::filesystem::path& p) { m_AddonsDirectory = p; }
        const std::filesystem::path& GetAddonDirectory() const { return m_AddonsDirectory; }

        void LoadAddons();           // scan dir and load new ones
        void UnloadAll();
        bool UnloadIndex(size_t i);  // unload one
        const std::vector<LoadedAddon>& Items() const { return m_Loaded; }

    private:
        static bool IsSharedLibraryFile(const std::filesystem::path& p);

        int m_AddonIdCounter = 0;
        AddonLoader m_Loader{};
        std::vector<LoadedAddon> m_Loaded;
        std::filesystem::path m_AddonsDirectory;
    };
}