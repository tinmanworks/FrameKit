// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Addon/AddonManager.cpp
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-01
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        
// =============================================================================

#include "FrameKit/Addon/AddonManager.h"
#include "FrameKit/Debug/Log.h"

#include <algorithm>


namespace FrameKit {

    AddonManager::AddonManager(IAddonPolicy& p) : policy_(p), loader_(*this) {}

    void AddonManager::SetDirectory(std::filesystem::path p) { dir_ = std::move(p); }

    void AddonManager::LoadAll() {
        items_.clear();
        if (dir_.empty()) return;
        for (auto& e : std::filesystem::directory_iterator(dir_)) {
            if (!e.is_regular_file()) continue;
            if (!policy_.IsAddonFile(e.path())) continue;
            if (auto ld = loader_.Load(e.path())) {
                policy_.OnAddonLoaded(*ld);
                items_.push_back(*ld);
            }
        }
    }

    void AddonManager::UnloadAll() {
        for (auto& a : items_) loader_.Unload(a);
        items_.clear();
    }

    void AddonManager::TickUpdate() { for (auto& a : items_) if (a.addon_v1 && a.addon_v1->OnUpdate) a.addon_v1->OnUpdate(); }
    void AddonManager::TickRender() { for (auto& a : items_) if (a.addon_v1 && a.addon_v1->OnRender) a.addon_v1->OnRender(); }
    void AddonManager::TickCyclic() { for (auto& a : items_) if (a.addon_v1 && a.addon_v1->OnCyclic) a.addon_v1->OnCyclic(); }


    void* AddonManager::HostGet(const char* id, uint32_t min_ver) noexcept {
        for (auto& h : host_ifaces_) if (std::string_view(h.id) == id && h.ver >= min_ver) return const_cast<void*>(h.table);
        return nullptr;
    }

    void AddonManager::RegisterHostInterface(const char* id, uint32_t ver, const void* table) {
        host_ifaces_.push_back(HostEntry{ id, ver, table });
    }

} // namespace FrameKit