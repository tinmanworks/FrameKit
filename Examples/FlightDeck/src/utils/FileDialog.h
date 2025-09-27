// FileDialog.h
#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace ui {

struct FileDialog {
    bool open = false;
    std::filesystem::path cwd{std::filesystem::current_path()};
    std::string selected;                         // full path of current selection
    std::vector<std::string> filters;             // e.g. {"mp4","mkv","avi"}

    // returns true if user confirmed and writes the chosen absolute path
    bool Show(const char* title, std::string& outPath);

    // extension filter helper
    static bool match_ext(const std::filesystem::path& p,
                          const std::vector<std::string>& exts);
};

} // namespace ui
