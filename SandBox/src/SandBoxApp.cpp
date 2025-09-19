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

#include "DemoLayer.h"
#include "VideoLayer.h"

#if defined(FK_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(FK_PLATFORM_LINUX)
#include <unistd.h>
#endif


namespace SandBox {

    class SandBoxApp : public FrameKit::Application {
    public:
        explicit SandBoxApp(const FrameKit::ApplicationSpecification& spec)
            : FrameKit::Application(spec) {
            FrameKit::Log::InitClient("SandBox Application");
            FrameKit::Log::GetClientLogger()->set_level(FrameKit::LogLevel::Info);

            FrameKit::InitializeWindowBackends();
        }

        bool Init() override {
            FK_PROFILE_FUNCTION();

            auto backends = FrameKit::ListWindowBackends();
            if (backends.empty()) {
                FK_WARN("No window backends registered");
            }
            else {
                FK_INFO("FDViz: window backends registered: {}", backends.size());
                for (const auto& b : backends) {
                    FK_INFO("Api={} priority={} name={}",
                        FrameKit::ToString(b.id), b.priority, b.name);
                }
            }
            
            PushLayer(new VideoLayer("DroneVideoPort"));
            PushLayer(new DemoLayer("DemoLayer"));

            FK_INFO("SandBox Application initialized");
            return true;
        }

        void Shutdown() override {
            FK_INFO("SandBoxApp shutting down");
        }
    };

} // namespace SandBox

FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name = "SandBoxApp";
    spec.WorkingDirectory = ".";
    spec.CommandLineArgs = args;
    spec.Mode = FrameKit::AppMode::Windowed;
    spec.WinSettings.api = FrameKit::WindowAPI::GLFW;
    spec.WinSettings.title = "SandBoxApp";
    spec.WinSettings.width = 1280;
    spec.WinSettings.height = 720;
    spec.WinSettings.vsync = true;
    spec.WorkingDirectory = std::filesystem::current_path();
     //spec.Master = false; // optional for multi-instance/IPC roles

    return new SandBox::SandBoxApp(spec);
}
