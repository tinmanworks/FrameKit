// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Addon/AddonManager.h
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Addon directory scanning, loading, ticking, and host interface registry.
// =============================================================================

#pragma once

#include "FrameKit/Addon/AddonLoader.h"
#include <filesystem>
#include <vector>
#include <memory>

namespace FrameKit {

    struct IAddonPolicy {
        virtual ~IAddonPolicy() = default;
        virtual bool IsAddonFile(const std::filesystem::path&) const = 0; // e.g. ".sae"
        virtual void OnAddonLoaded(LoadedAddon&) = 0;                     // app can query its own ifaces
    };

    class AddonManager : public IHostGetProvider {
    public:
        explicit AddonManager(IAddonPolicy& policy);

        // Directory-based ops
        void SetDirectory(std::filesystem::path p);
        void LoadAll();
        void UnloadAll();
        
        // Per-file ops
        bool LoadFile(const std::filesystem::path& p);
        bool UnloadFile(const std::filesystem::path& p);
        bool ReloadFile(const std::filesystem::path& p);
        bool IsLoaded(const std::filesystem::path& p) const;

        // Ticking
        void TickUpdate();
        void TickRender();
        void TickCyclic();

        const std::vector<LoadedAddon>& Items() const { return items_; }

        // IHostGetProvider
        void* HostGet(const char* id, uint32_t min_ver) noexcept override;

        // Registration for host tables
        void RegisterHostInterface(const char* id, uint32_t ver, const void* table);

    private:
        static std::string CanonicalKey(const std::filesystem::path& p);

        std::filesystem::path    dir_;
        IAddonPolicy&            policy_;
        AddonLoader              loader_;
        std::vector<LoadedAddon> items_;

        struct HostEntry { const char* id; uint32_t ver; const void* table; };
        std::vector<HostEntry> host_ifaces_;
    };
}
