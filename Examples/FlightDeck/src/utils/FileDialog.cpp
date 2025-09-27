// FileDialog.cpp
#include "FileDialog.h"
#include <imgui.h>
#include <imgui_stdlib.h> 
#include <algorithm>
#include <cctype>

using namespace std;
namespace fs = std::filesystem;

namespace {

// case-insensitive string equality
bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        unsigned char x = static_cast<unsigned char>(a[i]);
        unsigned char y = static_cast<unsigned char>(b[i]);
        if (std::tolower(x) != std::tolower(y)) return false;
    }
    return true;
}

} // namespace

namespace ui {

bool FileDialog::match_ext(const fs::path& p, const std::vector<std::string>& exts) {
    if (exts.empty()) return true;
    std::string e = p.extension().string();   // ".mp4"
    if (!e.empty() && e.front() == '.') e.erase(0, 1); // "mp4"
    for (const auto& x : exts) {
        if (!x.empty() && iequals(e, x)) return true;
    }
    return false;
}

bool FileDialog::Show(const char* title, std::string& outPath) {
    if (!open) return false;

    bool confirmed = false;
    ImGui::SetNextWindowSize(ImVec2(720, 420), ImGuiCond_FirstUseEver);

    if (ImGui::BeginPopupModal(title, &open, ImGuiWindowFlags_NoScrollbar)) {
        // Top bar: path + up
        const std::string cwdStr = cwd.string();      // native encoding; UTF-8 on Linux
        ImGui::TextUnformatted(cwdStr.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Up") && cwd.has_parent_path()) {
            cwd = cwd.parent_path();
            selected.clear();
        }
        ImGui::Separator();

        // Collect entries
        struct Entry { fs::path p; bool isDir; };
        std::vector<Entry> entries;
        try {
            for (auto& de : fs::directory_iterator(cwd)) {
                const bool isDir = de.is_directory();
                if (!isDir && !match_ext(de.path(), filters)) continue;
                entries.push_back({de.path(), isDir});
            }
        } catch (...) {
            // ignore permission or IO errors
        }

        std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b){
            if (a.isDir != b.isDir) return a.isDir > b.isDir; // dirs first
            return a.p.filename().string() < b.p.filename().string();
        });

        // List
        ImGui::BeginChild("list", ImVec2(0, -60), true, ImGuiWindowFlags_HorizontalScrollbar);

        if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0) && cwd.has_parent_path()) {
                cwd = cwd.parent_path();
                selected.clear();
            }
        }

        for (const auto& e : entries) {
            const bool sel = (!selected.empty() && fs::path(selected) == e.p);
            std::string label = e.isDir ? "[DIR] " : "      ";
            label += e.p.filename().string();
            if (ImGui::Selectable(label.c_str(), sel, ImGuiSelectableFlags_AllowDoubleClick)) {
                selected = e.p.string();
                if (ImGui::IsMouseDoubleClicked(0) && e.isDir) {
                    cwd = e.p;
                    selected.clear();
                }
            }
        }
        ImGui::EndChild();

        // Bottom bar: filename + buttons
        static std::string nameBuf;
        if (!selected.empty()) nameBuf = fs::path(selected).filename().string();

        ImGui::SetNextItemWidth(-200);
        ImGui::InputText("##name", &nameBuf);

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            open = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        const fs::path candidate = cwd / nameBuf;
        const bool canOpen = !nameBuf.empty() && fs::exists(candidate) && fs::is_regular_file(candidate);

        ImGui::BeginDisabled(!canOpen);
        if (ImGui::Button("Open")) {
            outPath = candidate.string();
            confirmed = true;
            open = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::EndPopup();
    }

    return confirmed;
}

} // namespace ui
