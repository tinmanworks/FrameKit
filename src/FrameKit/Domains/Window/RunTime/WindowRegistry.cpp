// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/RunTime/WindowRegistry.cpp
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Window backend registry
// =============================================================================

#include "FrameKit/Window/IWindow.h"
#include "FrameKit/Debug/Log.h"
#include "FrameKit/Debug/Instrumentor.h"

#include <unordered_map>
#include <mutex>
#include <type_traits>
#include <limits>

namespace FrameKit {

    namespace {
        struct Entry { std::string name; CreateWindowFn fn; int prio = 0; };

        struct EnumClassHash {
            template<class E>
            size_t operator()(E e) const noexcept {
                using U = std::underlying_type_t<E>;
                return std::hash<U>{}(static_cast<U>(e));
            }
        };

        std::mutex g_mx;
        std::unordered_map<WindowAPI, Entry, EnumClassHash> g_map;
    } // namespace


    bool RegisterWindowBackend(WindowAPI id, std::string_view name, CreateWindowFn fn, int priority) {
        std::lock_guard<std::mutex> lk(g_mx);
        auto& e = g_map[id];
        if (e.fn && e.prio >= priority) return false; // tie keeps first
        e = Entry{ std::string(name), std::move(fn), priority };
        return true;
    }

    std::vector<WindowAPIInfo> ListWindowBackends() {
        std::lock_guard<std::mutex> lk(g_mx);
        std::vector<WindowAPIInfo> out;
        out.reserve(g_map.size());
        for (auto& [id, e] : g_map) out.push_back(WindowAPIInfo{ id, e.name, e.prio });
        return out;
    }

    WindowPtr CreateWindow(WindowAPI id, const WindowDesc& d) {
		FK_PROFILE_FUNCTION();
        std::lock_guard<std::mutex> lk(g_mx);
        if (id == WindowAPI::Auto) {
            int bestPrio = std::numeric_limits<int>::min();
            WindowAPI bestId = WindowAPI::Auto;
            for (WindowAPI cand : {WindowAPI::GLFW, WindowAPI::Win32, WindowAPI::Cocoa}) {
                auto it = g_map.find(cand);
                if (it == g_map.end() || !it->second.fn) continue;
                if (it->second.prio > bestPrio) { bestPrio = it->second.prio; bestId = cand; }
            }
            if (bestId != WindowAPI::Auto) {
                FK_CORE_INFO("Selected Window Backend: {}", ToString(bestId));
                return g_map[bestId].fn(d);
            }
			FK_CORE_ERROR("No valid window backend found for 'Auto' selection");
            return WindowPtr(nullptr, +[](IWindow*) {});
        }
        auto it = g_map.find(id);
        return (it != g_map.end() && it->second.fn)
            ? it->second.fn(d)
            : WindowPtr(nullptr, +[](IWindow*) {});
    }

} // namespace FrameKit
