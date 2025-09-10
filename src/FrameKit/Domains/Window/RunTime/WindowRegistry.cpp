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
        std::unordered_map<WindowBackend, Entry, EnumClassHash> g_map;
    }

    bool RegisterWindowBackend(WindowBackend id, std::string_view name, CreateWindowFn fn, int priority) {
        std::lock_guard<std::mutex> lk(g_mx);
        auto& e = g_map[id];
        if (e.fn && e.prio >= priority) return false; // tie keeps first
        e = Entry{ std::string(name), std::move(fn), priority };
        return true;
    }

    std::vector<WindowBackendInfo> ListWindowBackends() {
        std::lock_guard<std::mutex> lk(g_mx);
        std::vector<WindowBackendInfo> out;
        out.reserve(g_map.size());
        for (auto& [id, e] : g_map) out.push_back(WindowBackendInfo{ id, e.name, e.prio });
        return out;
    }

    std::unique_ptr<IWindow> CreateWindow(WindowBackend id, const WindowDesc& d) {
        std::lock_guard<std::mutex> lk(g_mx);
        if (id == WindowBackend::Auto) {
            int bestPrio = std::numeric_limits<int>::min();
            WindowBackend bestId = WindowBackend::Auto;
            // deterministic tie-break by enum order
            for (WindowBackend cand : {WindowBackend::GLFW, WindowBackend::Win32, WindowBackend::Cocoa}) {
                auto it = g_map.find(cand);
                if (it == g_map.end() || !it->second.fn) continue;
                if (it->second.prio > bestPrio) { bestPrio = it->second.prio; bestId = cand; }
            }
            if (bestId != WindowBackend::Auto) return g_map[bestId].fn(d);
            return {};
        }
        auto it = g_map.find(id);
        return (it != g_map.end() && it->second.fn) ? it->second.fn(d) : std::unique_ptr<IWindow>{};
    }

} // namespace FrameKit
