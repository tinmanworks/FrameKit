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

#define FK_WINDOW_BACKEND_GLFW_ENABLE
#include <FrameKit/Window.h>

#include "DemoLayer.h"

namespace SandBox {

    class SandBoxApp : public FrameKit::Application {
    public:
        explicit SandBoxApp(const FrameKit::ApplicationSpecification& spec)
            : FrameKit::Application(spec) {
            FrameKit::Log::InitClient("SandBoxApp");
            FrameKit::Log::GetClientLogger()->set_level(FrameKit::LogLevel::Info);
        }

        bool Init() override {
            FK_PROFILE_FUNCTION();

            FrameKit::InitializeWindowBackends();

            auto backends = FrameKit::ListWindowBackends();
            if (backends.empty()) {
                FK_WARN("No window backends registered");
            }
            else {
                FK_INFO("Window backends registered: {}", backends.size());
                for (const auto& b : backends) {
                    FK_INFO("api={} priority={} name={}", FrameKit::ToString(b.id), b.priority, b.name);
                }
            }

            PushLayer(new DemoLayer("DemoLayer"));
            FK_INFO("SandBoxApp initialized");
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
    spec.CommandLineArgs = args;
    spec.Mode = FrameKit::AppMode::Windowed;
    spec.WinSettings.api = FrameKit::WindowAPI::Auto;
    spec.WinSettings.title = "SandBoxApp";
    spec.WinSettings.width = 1280;
    spec.WinSettings.height = 720;
    spec.WinSettings.vsync = true;
    spec.WorkingDirectory = std::filesystem::current_path();
    // spec.Master = false; // optional for multi-instance/IPC roles

    return new SandBox::SandBoxApp(spec);
}
