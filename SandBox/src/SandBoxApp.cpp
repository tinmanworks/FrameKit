
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
			printf("SandBoxApp::Shutdown\n");\
        }
    };

}

FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args) {

    ApplicationSpecification spec;
	spec.Name = "SandBoxApp";
	spec.CommandLineArgs = args;
	spec.AppMode = FrameKit::AppMode::Windowed;

    return new SandBox::SandBoxApp(spec);
}