// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/RunTime/WindowPluginLoader.cpp
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Dynamically loads window backend plugins
// =============================================================================

#include "FrameKit/Window/Window.h"
#include "FrameKit/Window/WindowPlugin.h"

#include <filesystem>
#include <windows.h>
#include <mutex>

#if defined(FK_PLATFORM_WINDOWS)
#include <windows.h>
static void* loadLib(const std::filesystem::path& p) { return (void*)LoadLibraryW(p.wstring().c_str()); }
static void* loadSym(void* h, const char* n) { return (void*)GetProcAddress((HMODULE)h, n); }
static void  closeLib(void* h) { if (h) FreeLibrary((HMODULE)h); }
#elif defined(FK_PLATFORM_APPLE)
#include <dlfcn.h>
static void* loadLib(const std::filesystem::path& p) { return dlopen(p.string().c_str(), RTLD_NOW); }
static void* loadSym(void* h, const char* n) { return dlsym(h, n); }
static void  closeLib(void* h) { if (h) dlclose(h); }
#else
#include <dlfcn.h>
static void* loadLib(const std::filesystem::path& p) { return dlopen(p.string().c_str(), RTLD_NOW); }
static void* loadSym(void* h, const char* n) { return dlsym(h, n); }
static void  closeLib(void* h) { if (h) dlclose(h); }
#endif

namespace FrameKit {
    namespace fs = std::filesystem;

    namespace {
        struct LoadedLib { void* handle{}; std::string name; int id{}; const FrameKit_WindowPlugin* plug{}; };
        static std::vector<LoadedLib> g_loaded; // keep modules resident
        // map each created window to its plugin destroy function
        static std::mutex g_owner_mx;
        static std::unordered_map<IWindow*, const FrameKit_WindowPlugin*> g_owner;

        static void DeleteWindowNonCapturing(IWindow* w) noexcept {
            if (!w) return;
            const FrameKit_WindowPlugin* P = nullptr;
            {
                std::lock_guard<std::mutex> lk(g_owner_mx);
                auto it = g_owner.find(w);
                if (it != g_owner.end()) { P = it->second; g_owner.erase(it); }
            }
            if (P && P->destroy) P->destroy(reinterpret_cast<void*>(w));

        }
    }

    namespace {

        inline WindowPtr MakePtr(const FrameKit_WindowPlugin* P, IWindow* raw) {
            auto del = [](IWindow* w) { /* placeholder, replaced below */ };
            // supply a noncapturing deleter pointer by binding P->destroy through a static shim:
            // but since CreateWindowFn is std::function now, we can keep a capturing deleter:
            struct Ctx {
                const FrameKit_WindowPlugin* P;
                static void call(void* ctx, IWindow* w) noexcept {
                    auto* c = static_cast<const Ctx*>(ctx);
                    if (w && c && c->P && c->P->destroy) c->P->destroy(reinterpret_cast<void*>(w));
                }
            };

            // type-erased storage for deleter target
            static_assert(sizeof(void(*)(IWindow*)) == sizeof(void*), "assumes pointer size");

            // Build a function-pointer style deleter that closes over P via a static registry.
            // Simpler: store a table indexed by raw pointer. Here we avoid that—return with a capturing std::function instead by using WindowPtr alias above.
            return { raw, +[](IWindow* w) { /* unused when CreateWindowFn is std::function */ } };
        }

    } // anon

    // Create with function-pointer deleter; associate window → plugin in registry
    static WindowPtr WrapC(const FrameKit_WindowPlugin* P, const WindowDesc& d) {
        FrameKit_WindowDescC cd{ d.title.c_str(), d.width, d.height, d.resizable, d.vsync, d.visible, d.highDPI };
        auto* raw = reinterpret_cast<IWindow*>(P->create(&cd));
        {
            std::lock_guard<std::mutex> lk(g_owner_mx);
            if (raw) g_owner.emplace(raw, P);
        }
        return WindowPtr(raw, &DeleteWindowNonCapturing);
    }

    void LoadWindowPlugin(const fs::path& libPath) {
        void* h = loadLib(libPath);
        if (!h) return;
        using GetFn = const FrameKit_WindowPlugin* (*)();
        auto sym = reinterpret_cast<GetFn>(loadSym(h, "FrameKit_GetWindowPlugin"));
        if (!sym) { closeLib(h); return; }
        const auto* P = sym();
        if (!P || P->abi != FRAMEKIT_WINDOW_PLUGIN_ABI || !P->create || !P->destroy) { closeLib(h); return; }

        auto thunk = [P](const WindowDesc& d) { return WrapC(P, d); }; // capturing OK (std::function)
        const auto id = static_cast<WindowBackend>(P->id);
        if (RegisterWindowBackend(id, P->name ? P->name : "Plugin", std::move(thunk), 200)) {
            g_loaded.push_back(LoadedLib{ h, P->name ? P->name : "Plugin", P->id, P });
        }
        else {
            closeLib(h);
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