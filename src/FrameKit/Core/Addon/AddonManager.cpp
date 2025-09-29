// AddonManager.cpp
#include "FrameKit/Addon/AddonManager.h"

#include "FrameKit/Debug/Log.h"

#include <iostream>
#include <system_error>

namespace FrameKit {

    bool AddonManager::IsSharedLibraryFile(const std::filesystem::path& p) {
        if (!p.has_extension()) return false;

#if defined(FK_PLATFORM_WINDOWS)
        auto ext = p.extension().wstring();
        return _wcsicmp(ext.c_str(), L".dll") == 0;
#else // Linux and other UNIX
        auto ext = p.extension().string();
        for (auto& c : ext) c = static_cast<char>(std::tolower(c));
        return ext == ".so";
#endif
}

    void AddonManager::LoadAddons() {
        std::error_code ec;
        if (m_AddonsDirectory.empty() ||
            !std::filesystem::exists(m_AddonsDirectory, ec) ||
            !std::filesystem::is_directory(m_AddonsDirectory, ec)) return;

        for (const auto& ent : std::filesystem::directory_iterator(m_AddonsDirectory, ec)) {
            if (ec || !ent.is_regular_file()) continue;
            const auto& p = ent.path();
            if (!IsSharedLibraryFile(p)) continue;

            // skip if already loaded
            bool dup = false;
            for (auto& e : m_Loaded) if (std::filesystem::equivalent(e.path, p, ec)) { dup = true; break; }
            if (dup) continue;

            try {
                printf(p.string().c_str());
                printf("\n");
                IAddonHost* addonHostptr = m_Loader.LoadAddon(p);
                LoadedAddon rec{};
                rec.Id = ++m_AddonIdCounter;
                rec.path = p;
                rec.host = addonHostptr;
                rec.tag = addonHostptr->ahb_fns.get_tag();
                rec.abi = addonHostptr->abi;
                m_Loaded.push_back(rec);
            }
            catch (...) {
                FK_CORE_WARN("Failed to load Addon {}", ent.path().string());
            }
        }
    }

    bool AddonManager::UnloadIndex(size_t i) {
        if (i >= m_Loaded.size()) return false;
        auto h = m_Loaded[i].host;
        if (h) { m_Loader.UnloadAddon(*h); }
        m_Loaded.erase(m_Loaded.begin() + static_cast<std::ptrdiff_t>(i));
        return true;
    }

    void AddonManager::UnloadAll() {
        for (auto& e : m_Loaded) if (e.host) m_Loader.UnloadAddon(*e.host);
        m_Loaded.clear();
    }

} // namespace FrameKit