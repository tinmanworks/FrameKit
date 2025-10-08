// =============================================================================
// Project      : FlightDeck
// File         : src/AddonManagerLayer.cpp
// Author       : George Gil
// Created      : 2025-10-08
// Updated      : 2025-10-08
// License      : Application code using FrameKit
// Description  : Implementation of AddonManagerLayer with an ImGui control panel
//                to choose directory, scan, and load/unload addons.
// =============================================================================
#include "AddonManagerLayer.h"
#include <imgui.h>
#include <algorithm>

namespace FlightDeck {

    bool FD_AddonPolicy::IsAddonFile(const std::filesystem::path& p) const {
        auto e = p.extension().string();
        std::transform(e.begin(), e.end(), e.begin(), ::tolower);
        return e == ".sae";
    }

    void FD_AddonPolicy::OnAddonLoaded(FrameKit::LoadedAddon& /*a*/) {
        // Optionally query app-specific addon interfaces here.
    }

    AddonManagerLayer::AddonManagerLayer(std::filesystem::path addonsDir, HostRegFn reg)
        : FrameKit::Layer("AddonManagerLayer")
        , m_Manager(FrameKit::CreateRef<FrameKit::AddonManager>(m_Policy))
        , m_AddonsDir(std::move(addonsDir))
        , m_Reg(std::move(reg)) {
        std::snprintf(m_PathBuf, sizeof(m_PathBuf), "%s", m_AddonsDir.string().c_str());
    }

    void AddonManagerLayer::OnAttach() {
        m_Manager->SetDirectory(m_AddonsDir);
        if (m_Reg) m_Reg(*m_Manager);
        if (m_AutoLoadOnAttach) {
            m_Manager->LoadAll();
            m_Loaded = true;
        }
        ScanDirectory();
    }

    void AddonManagerLayer::OnDetach() {
        if (m_Loaded) m_Manager->UnloadAll();
        m_Loaded = false;
        m_Found.clear();
    }

    void AddonManagerLayer::OnSyncUpdate(FrameKit::Timestep) {
        // Drive addon lifecycle around your app�s frame
        m_Manager->TickUpdate();
    }

    void AddonManagerLayer::OnRender() {
        m_Manager->TickRender();
        DrawUI();
    }

    void AddonManagerLayer::OnAsyncUpdate() {
        m_Manager->TickCyclic();
    }

    void AddonManagerLayer::ScanDirectory() {
        m_Found.clear();
        std::error_code ec;
        if (!std::filesystem::exists(m_AddonsDir, ec)) return;
        for (auto& e : std::filesystem::directory_iterator(m_AddonsDir, ec)) {
            if (!e.is_regular_file()) continue;
            if (!m_Policy.IsAddonFile(e.path())) continue;
            m_Found.push_back(FoundItem{ e.path() });
        }
        std::sort(m_Found.begin(), m_Found.end(),
            [](const FoundItem& a, const FoundItem& b) { return a.path.filename() < b.path.filename(); });
    }

    void AddonManagerLayer::DrawUI() {
        if (!ImGui::Begin("Addons")) { ImGui::End(); return; }

        // Directory selector
        ImGui::InputText("Folder", m_PathBuf, sizeof(m_PathBuf));
        ImGui::SameLine();
        if (ImGui::Button("Set")) {
            m_AddonsDir = std::filesystem::path(m_PathBuf);
            m_Manager->SetDirectory(m_AddonsDir);
            ScanDirectory();
        }
        ImGui::SameLine();
        if (ImGui::Button("Scan")) {
            ScanDirectory();
        }

        // Actions
        if (ImGui::Button("Load All")) {
            m_Manager->LoadAll();
            m_Loaded = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Unload All")) {
            m_Manager->UnloadAll();
            m_Loaded = false;
        }

        ImGui::Separator();

        // Found files list
        ImGui::TextUnformatted("Discovered plugins:");
        if (m_Found.empty()) {
            ImGui::TextDisabled("none");
        }
        else {
            for (const auto& f : m_Found) {
                ImGui::BulletText("%s", f.path.filename().string().c_str());
            }
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Loaded addons:");
        // We only expose aggregate info via Items(). Add accessor in manager if missing.
        // Show names if addons reported them during load.
        const auto& items = m_Manager->Items();
        if (items.empty()) {
            ImGui::TextDisabled("none");
        }
        else {
            for (const auto& a : items) {
                const char* name = a.info.name ? a.info.name : "(unnamed)";
                ImGui::BulletText("%s", name);
            }
        }

        ImGui::End();
    }

} // namespace FlightDeck
