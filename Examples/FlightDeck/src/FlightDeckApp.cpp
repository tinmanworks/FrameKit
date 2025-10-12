// =============================================================================
// Project      : FlightDeck
// File         : src/FlightDeckApp.cpp
// Author       : George Gil
// Created      : 2025-09-18
// Updated      : 2025-10-08
// License      : Application code using FrameKit
// Description  :
//      Example client application using FrameKit. Wires the AddonManagerLayer
//      with context-bearing ABI, registers host service tables, loads .sae
//      addons from ./Addons, and ticks them each frame alongside ImGui + Video.
// =============================================================================

#include <FrameKit/FrameKit.h>

#define FK_WINDOW_BACKEND_GLFW_ENABLE 1
#include <FrameKit/Window.h>

#include <FrameKit/Addon/AddonManager.h>
#include <FrameKit/Addon/FKHostV1.h>
#include "FDAExt.h"

// #include "VideoLayer.h"
#include "ImGui/ImGuiLayer.h"
#include "AddonManagerLayer.h"

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <filesystem>

#if defined(FK_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(FK_PLATFORM_LINUX)
#include <unistd.h>
#endif

namespace FlightDeck {

    // FK_HostV1 services
    static void* HAlloc(uint64_t n) noexcept { return std::malloc((size_t)n); }
    static void  HFree(void* p) noexcept { std::free(p); }
    static void  HLog(int lvl, const char* m) noexcept { std::printf("[FlightDeck L%d] %s\n", lvl, m ? m : ""); }
    static double HNow() noexcept {
        using namespace std::chrono;
        return duration<double>(steady_clock::now().time_since_epoch()).count();
    }
    static FK_HostV1 g_fk_host{ 1u, sizeof(FK_HostV1), &HAlloc, &HFree, &HLog, &HNow };

    static void* GetImGuiCtx() noexcept { return ImGui::GetCurrentContext(); }
    static SB_ImGuiHostV1 g_imgui_host{1u, sizeof(SB_ImGuiHostV1), &GetImGuiCtx};

    // Sandbox host extras (optional)
    static void        SBSetImGui(void*) noexcept {}
    static const char* SBGetDoc() noexcept { return "Untitled"; }

    class FlightDeckApp : public FrameKit::Application {
    public:
        explicit FlightDeckApp(const FrameKit::ApplicationSpecification& spec)
            : FrameKit::Application(spec) {
            FrameKit::Log::InitClient("FlightDeck Application");
            FrameKit::Log::GetClientLogger()->set_level(FrameKit::LogLevel::Info);
            FrameKit::InitializeWindowBackends();
        }

        bool Init() override {
            FK_PROFILE_FUNCTION();

            // Layers
            m_ImGuiLayer = new ImGuiLayer();
            PushLayer(m_ImGuiLayer);
            // PushLayer(new VideoLayer("DroneVideoPort"));

            // Addon manager layer with host registration lambda
            auto addonsDir = std::filesystem::current_path();
            auto* amLayer = new AddonManagerLayer(
                addonsDir,
                [](FrameKit::AddonManager& mgr){
                    mgr.RegisterHostInterface(FK_IFACE_HOST_V1,      1, &g_fk_host);
                    mgr.RegisterHostInterface(SB_IFACE_IMGUI_HOST_V1,1, &g_imgui_host);
                    // keep SB_IFACE_HOST_V1 if you also use it
                });
            PushLayer(amLayer);

            FK_INFO("FlightDeck Application initialized");
            return true;
        }

        void OnBeforeRender() override { m_ImGuiLayer->Begin(); }
        void OnAfterRender()  override { m_ImGuiLayer->End(); }

    private:
        ImGuiLayer* m_ImGuiLayer{ nullptr };
    };

} // namespace FlightDeck

FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name = "FlightDeckApp";
    spec.WorkingDirectory = std::filesystem::current_path();
    spec.CommandLineArgs = args;
    spec.Mode = FrameKit::AppMode::Windowed;
    spec.WinSettings.api = FrameKit::WindowAPI::GLFW;
    spec.WinSettings.title = "FlightDeckApp";
    spec.WinSettings.width = 1280;
    spec.WinSettings.height = 720;
    spec.WinSettings.vsync = true;
    return new FlightDeck::FlightDeckApp(spec);
}
