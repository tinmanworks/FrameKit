// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/RunTime/WindowRegistry.cpp
// Author       : George Gil
// Created      : 2025-09-18
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Window registry
// =============================================================================

#include "FrameKit/Window/WindowRegistry.h"

#include <unordered_map>
#include <mutex>
#include <atomic>

namespace FrameKit {
static std::mutex g_mx;
static std::unordered_map<IWindow*, WindowInfo> g_byPtr;
static std::unordered_map<WindowId, IWindow*>   g_byId;
static std::atomic<WindowId> g_next{1};

WindowId WindowRegistry::Register(IWindow* w, WindowAPI api, std::string name){
    if(!w) return 0;
    std::lock_guard<std::mutex> lk(g_mx);
    if (auto it = g_byPtr.find(w); it != g_byPtr.end()) return it->second.id;
    WindowId id = g_next++;
    g_byPtr.emplace(w, WindowInfo{id,w,api,std::move(name)});
    g_byId[id] = w;
    return id;
}
void WindowRegistry::Unregister(IWindow* w) noexcept{
    if(!w) return;
    std::lock_guard<std::mutex> lk(g_mx);
    auto it = g_byPtr.find(w);
    if(it==g_byPtr.end()) return;
    g_byId.erase(it->second.id);
    g_byPtr.erase(it);
}
IWindow* WindowRegistry::Get(WindowId id) noexcept{
    std::lock_guard<std::mutex> lk(g_mx);
    auto it = g_byId.find(id);
    return it==g_byId.end()? nullptr : it->second;
}
std::vector<WindowInfo> WindowRegistry::List(){
    std::lock_guard<std::mutex> lk(g_mx);
    std::vector<WindowInfo> v; v.reserve(g_byPtr.size());
    for(auto& kv: g_byPtr) v.push_back(kv.second);
    return v;
}
}
