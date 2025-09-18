#include <FrameKit/FrameKit.h>

#define FK_WINDOW_BACKEND_GLFW_ENABLE
#include <FrameKit/Window.h>

#include "DemoLayer.h"

#include <iostream>

namespace SandBox {

    class SandBoxApp : public FrameKit::Application {
    public:
        explicit SandBoxApp(const FrameKit::ApplicationSpecification& spec)
            : FrameKit::Application(spec) {
            FrameKit::Log::InitClient("SandBoxApp");
        }

        bool Init() override {
			FK_PROFILE_FUNCTION();
            FrameKit::InitializeWindowBackends();
			PushLayer(new DemoLayer("DemoLayer"));
			FK_INFO("SandBoxApp Initialized");
            return true;
        }

        void Shutdown() override {
            FK_INFO("SandBoxApp Shutting down");
        }
    };

}

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
	// spec.Master = false; // optional, for multi-instance apps or IPC roles

    return new SandBox::SandBoxApp(spec);
}