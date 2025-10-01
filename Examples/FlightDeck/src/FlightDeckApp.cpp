// =============================================================================
// Project      : SandBox
// File         : SandBoxApp.cpp
// Author       : George Gil
// Created      : 2025-09-18
// License      : Application code using FrameKit
// Description  :
//      Example client application. Initializes window backends,
//      pushes a demo layer, and logs lifecycle events.
// =============================================================================

#include <FrameKit/FrameKit.h>

#define FK_WINDOW_BACKEND_GLFW_ENABLE 1
#include <FrameKit/Window.h>

#include <FrameKit/Addon/AddonManager.h>

#include "FDAExt.h"
#include "VideoLayer.h"
#include "ImGui/ImGuiLayer.h"


#if defined(FK_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(FK_PLATFORM_LINUX)
#include <unistd.h>
#endif


namespace FlightDeck {
    
    static void* HAlloc(uint64_t n) noexcept { return std::malloc((size_t)n); }
    static void  HFree(void* p) noexcept { std::free(p); }
    static void  HLog(int lvl, const char* m) noexcept { std::printf("[L%d] %s\n", lvl, m ? m : ""); }
    static double HNow() noexcept {
        using namespace std::chrono;
        return duration<double>(steady_clock::now().time_since_epoch()).count();
    }

    static void SBSetImGui(void*) noexcept {}

    static SB_HostV1 g_sb_host{
        1u, sizeof(SB_HostV1), &SBSetImGui
    };

    static FK_HostV1 g_fk_host{
    1u, sizeof(FK_HostV1), &HAlloc, &HFree, &HLog, &HNow
    };

    struct Policy : FrameKit::IAddonPolicy {
        bool IsAddonFile(const std::filesystem::path& p) const override {
            auto e = p.extension().string();
            std::transform(e.begin(), e.end(), e.begin(), ::tolower);
            return e == ".dll"; // FlightDeck Addon Extension
        }
        void OnAddonLoaded(FrameKit::LoadedAddon& a) override {
            // App can query addon-specific interfaces here if needed
            if (auto* img = (SB_ImGuiV1*)a.addon_get(SB_IFACE_IMGUI_V1, 1)) {
                // e.g., cache pointer somewhere or call SetContext
                (void)img;
            }
        }
    };

    class FlightDeckApp : public FrameKit::Application {
    public:
        explicit FlightDeckApp(const FrameKit::ApplicationSpecification& spec)
            : FrameKit::Application(spec) {
            FrameKit::Log::InitClient("FlightDeck Application");
            FrameKit::Log::GetClientLogger()->set_level(FrameKit::LogLevel::Info);

            FrameKit::InitializeWindowBackends();
            
			m_AddonManager = FrameKit::CreateRef<FrameKit::AddonManager>(m_Policy);

            // register host tables available to addons
            m_AddonManager->RegisterHostInterface(FK_IFACE_HOST_V1, 1, &g_fk_host);
            m_AddonManager->RegisterHostInterface(SB_IFACE_HOST_V1, 1, &g_sb_host);
        }

        bool Init() override {
            FK_PROFILE_FUNCTION();

            auto backends = FrameKit::ListWindowBackends();
            if (backends.empty()) {
                FK_WARN("No window backends registered");
            }
            else {
                FK_INFO("FlightDeck: window backends registered: {}", backends.size());
                for (const auto& b : backends) {
                    FK_INFO("Api={} priority={} name={}",
                        FrameKit::ToString(b.id), b.priority, b.name);
                }
            }
            
            m_ImGuiLayer = new ImGuiLayer();
            PushLayer(m_ImGuiLayer);

            PushLayer(new VideoLayer("DroneVideoPort"));

            FK_INFO("FlightDeck Application initialized");
            return true;
        }

        void OnBeforeRender() override { m_ImGuiLayer->Begin(); }

        void OnAfterRender() override { m_ImGuiLayer->End(); }

        void Shutdown() override {
            FK_INFO("FlightDeckApp shutting down");
        }

    private:
        Policy m_Policy;
        FrameKit::Ref<FrameKit::AddonManager> m_AddonManager;
        ImGuiLayer* m_ImGuiLayer;
    };

} // namespace FlightDeck

FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name = "FlightDeckApp";
    spec.WorkingDirectory = ".";
    spec.CommandLineArgs = args;
    spec.Mode = FrameKit::AppMode::Windowed;
    spec.WinSettings.api = FrameKit::WindowAPI::GLFW;
    spec.WinSettings.title = "FlightDeckApp";
    spec.WinSettings.width = 1280;
    spec.WinSettings.height = 720;
    spec.WinSettings.vsync = true;
    spec.WorkingDirectory = std::filesystem::current_path();
     //spec.Master = false; // optional for multi-instance/IPC roles

    return new FlightDeck::FlightDeckApp(spec);
}
