
#include "FrameKit/Application/Application.h"
#include "FrameKit/Debug/Log.h"
#include "FrameKit/Debug/Instrumentor.h"

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
	spec.WinSettings.api = FrameKit::WindowAPI::Win32; 
	spec.WinSettings.title = "SandBoxApp";
	spec.WinSettings.width = 1280;
	spec.WinSettings.height = 720;
	spec.WinSettings.vsync = true;
	spec.WorkingDirectory = std::filesystem::current_path();
	// spec.Master = false; // optional, for multi-instance apps or IPC roles

    return new SandBox::SandBoxApp(spec);
}