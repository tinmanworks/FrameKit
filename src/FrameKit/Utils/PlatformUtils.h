#pragma once

#include <string>
#include <chrono>
#include <filesystem>
namespace FrameKit {
    typedef uint64_t UnixTimestamp_t;

    namespace FileDialogs {

        std::string OpenFile(const char* filter);
        std::string OpenFile(const char* filter, const std::string& initialDir, void* hanlder = nullptr);
        std::string OpenFile(const char* filter, const std::filesystem::path& initialDir);
        std::string OpenFolder(const char* title = "", const std::string& initialDir = "");        
        std::string SaveFile(const char* filter, const std::string& initialDir = "");
    }

    //class FileDialogs {
    //public:
    //    // These return empty strings if cancelled
    //    static std::string OpenFile(const char* filter, const std::string& initialDir = "");
    //    static std::string OpenFolder(const char* title = "", const std::string& initialDir = "");
    //    static std::string SaveFile(const char* filter, const std::string& initialDir = "");
    //};

    class Time {
    public:
        static float GetTime() {

            static auto start = std::chrono::high_resolution_clock::now();
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = now - start;
            return elapsed.count();

        }
    };

}
