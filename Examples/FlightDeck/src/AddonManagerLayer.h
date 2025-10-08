// =============================================================================
// Project      : FlightDeck
// File         : src/AddonManagerLayer.h
// Author       : George Gil
// Created      : 2025-10-08
// Updated      : 2025-10-08
// License      : Application code using FrameKit
// Description  : Layer that encapsulates FrameKit AddonManager lifecycle
//                and provides a small ImGui UI to scan and load addons.
// =============================================================================
#pragma once

#include <FrameKit/FrameKit.h>
#include <FrameKit/Addon/AddonManager.h>
#include <filesystem>
#include <functional>
#include <vector>

namespace FlightDeck {

    struct FD_AddonPolicy final : FrameKit::IAddonPolicy {
        bool IsAddonFile(const std::filesystem::path& p) const override;
        void OnAddonLoaded(FrameKit::LoadedAddon& a) override;
    };

    class AddonManagerLayer : public FrameKit::Layer {
    public:
        using HostRegFn = std::function<void(FrameKit::AddonManager&)>;

        AddonManagerLayer(std::filesystem::path addonsDir, HostRegFn reg = {});
        ~AddonManagerLayer() override = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnSyncUpdate(FrameKit::Timestep) override;
        void OnRender() override;
        void OnAsyncUpdate() override;

        FrameKit::AddonManager& Manager() { return *m_Manager; }

    private:
        // UI helpers
        void ScanDirectory();
        void DrawUI();

        struct FoundItem {
            std::filesystem::path path;
        };

        FD_AddonPolicy m_Policy{};
        FrameKit::Ref<FrameKit::AddonManager> m_Manager;
        std::filesystem::path m_AddonsDir;
        HostRegFn m_Reg;
        bool m_Loaded{ false };

        // UI state
        std::vector<FoundItem> m_Found;
        char m_PathBuf[1024]{};
        bool m_AutoLoadOnAttach{ true };
    };

} // namespace FlightDeck
