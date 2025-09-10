// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Window/Window.h
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
// =============================================================================

#include "FrameKit/Window/Window.h"
#include "FrameKit/Window/WindowPlugin.h"

#include <filesystem>
#include <vector>
#include <string>

#if defined(_WIN32)
#include <windows.h>
static void* loadLib(const std::filesystem::path& p) { return (void*)LoadLibraryW(p.wstring().c_str()); }
static void* loadSym(void* h, const char* n) { return (void*)GetProcAddress((HMODULE)h, n); }
#elif defined(__APPLE__)
#include <dlfcn.h>
static void* loadLib(const std::filesystem::path& p) { return dlopen(p.string().c_str(), RTLD_NOW); }
static void* loadSym(void* h, const char* n) { return dlsym(h, n); }
#else
#include <dlfcn.h>
static void* loadLib(const std::filesystem::path& p) { return dlopen(p.string().c_str(), RTLD_NOW); }
static void* loadSym(void* h, const char* n) { return dlsym(h, n); }
#endif

namespace FrameKit {
    namespace fs = std::filesystem;

    namespace {
        struct LoadedLib { void* handle{}; std::string name; int id{}; const FrameKit_WindowPlugin* plug{}; };
        static std::vector<LoadedLib> g_loaded; // keep modules resident
    }

    // Internal helper. Requires plugin and host built compatibly.
    static std::unique_ptr<IWindow> WrapC(const FrameKit_WindowPlugin* P, const WindowDesc& d) {
        FrameKit_WindowDescC cd{ d.title.c_str(), d.width, d.height, d.resizable, d.vsync, d.visible, d.highDPI };
        auto raw = reinterpret_cast<IWindow*>(P->create(&cd));
        return std::unique_ptr<IWindow>(raw); // plugin must allocate with compatible CRT; plugin's destroy is unused here
    }

    void LoadWindowPlugin(const fs::path& libPath) {
        void* h = loadLib(libPath);
        if (!h) return;
        using GetFn = const FrameKit_WindowPlugin* (*)();
        auto sym = (GetFn)loadSym(h, "FrameKit_GetWindowPlugin");
        if (!sym) return;
        auto* P = sym();
        if (!P || P->abi != FRAMEKIT_WINDOW_PLUGIN_ABI || !P->create || !P->destroy) return;

        auto thunk = [P](const WindowDesc& d) { return WrapC(P, d); };
        auto id = static_cast<WindowBackend>(P->id);
        if (RegisterWindowBackend(id, P->name ? P->name : "Plugin", thunk, 200)) {
            g_loaded.push_back(LoadedLib{ h, P->name ? P->name : "Plugin", P->id, P });
        }
    }

    void LoadWindowPluginsFrom(const fs::path& dir) {
        if (!fs::exists(dir)) return;
#if defined(_WIN32)
        const char* ext = ".dll";
#elif defined(__APPLE__)
        const char* ext = ".dylib";
#else
        const char* ext = ".so";
#endif
        for (auto& e : fs::directory_iterator(dir)) {
            if (!e.is_regular_file()) continue;
            if (e.path().extension() == ext) LoadWindowPlugin(e.path());
        }
    }

} // namespace FrameKit
