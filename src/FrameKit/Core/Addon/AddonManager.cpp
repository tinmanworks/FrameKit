// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/Addon/AddonManager.cpp
// Author       : George Gil
// Created      : 2025-09-20
// Updated      : 2025-10-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//        Manager implementation: scan, load, tick, unload, host iface registry.
// =============================================================================

#include "FrameKit/Addon/AddonManager.h"
#include "FrameKit/Debug/Log.h"
#include <algorithm>
#include <string_view>

namespace FrameKit {
    // --- helpers --------------------------------------------------------------

    std::string AddonManager::CanonicalKey(const std::filesystem::path& p) {
        std::error_code ec;
        auto c = std::filesystem::weakly_canonical(p, ec);
        return (ec ? p : c).string();
    }
 
    // --- lifecycle ------------------------------------------------------------

    AddonManager::AddonManager(IAddonPolicy& p)
        : policy_(p)
        , loader_(*this) {}

    void AddonManager::SetDirectory(std::filesystem::path p) { dir_ = std::move(p); }

    void AddonManager::LoadAll() {
        items_.clear();
        if (dir_.empty()) return;

        std::error_code ec;
        if (!std::filesystem::exists(dir_, ec)) return;

        for (auto& e : std::filesystem::directory_iterator(dir_, ec)) {
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

    // --- per-file ops ---------------------------------------------------------

    bool AddonManager::IsLoaded(const std::filesystem::path& p) const {
        const auto key = CanonicalKey(p);
        return std::any_of(items_.begin(), items_.end(), [&](const LoadedAddon& a){
            return CanonicalKey(a.path) == key;
        });
    }

    bool AddonManager::LoadFile(const std::filesystem::path& p) {
        if (!policy_.IsAddonFile(p)) return false;
        if (IsLoaded(p)) return true;
        if (auto ld = loader_.Load(p)) {
            policy_.OnAddonLoaded(*ld);
            items_.push_back(*ld);
            return true;
        }
        return false;
    }

    bool AddonManager::UnloadFile(const std::filesystem::path& p) {
        const auto key = CanonicalKey(p);
        auto it = std::find_if(items_.begin(), items_.end(), [&](const LoadedAddon& a){
            return CanonicalKey(a.path) == key;
        });
        if (it == items_.end()) return false;
        loader_.Unload(*it);
        items_.erase(it);
        return true;
    }

    bool AddonManager::ReloadFile(const std::filesystem::path& p) {
        const bool had = IsLoaded(p);
        if (had) UnloadFile(p);
        return LoadFile(p);
    }

    // --- ticking --------------------------------------------------------------

    void AddonManager::TickUpdate() { for (auto& a : items_) if (a.addon_v1 && a.addon_v1->OnUpdate) a.addon_v1->OnUpdate(); }
    void AddonManager::TickRender() { for (auto& a : items_) if (a.addon_v1 && a.addon_v1->OnRender) a.addon_v1->OnRender(); }
    void AddonManager::TickCyclic() { for (auto& a : items_) if (a.addon_v1 && a.addon_v1->OnCyclic) a.addon_v1->OnCyclic(); }

    // --- host interfaces ------------------------------------------------------

    void* AddonManager::HostGet(const char* id, uint32_t min_ver) noexcept {
        for (auto& h : host_ifaces_)
            if (std::string_view(h.id) == id && h.ver >= min_ver)
                return const_cast<void*>(h.table);
        return nullptr;
    }

    void AddonManager::RegisterHostInterface(const char* id, uint32_t ver, const void* table) {
        host_ifaces_.push_back(HostEntry{ id, ver, table });
    }

} // namespace FrameKit