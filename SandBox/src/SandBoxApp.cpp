
#include "FrameKit/Application/Application.h"

#include "DemoLayer.h"

#include <iostream>

namespace SandBox {

    class SandBoxApp : public FrameKit::Application {
    public:
        explicit SandBoxApp(const FrameKit::ApplicationSpecification& spec)
            : FrameKit::Application(spec) {}

        bool Init() override {
			printf("SandBoxApp::Init\n");
			PushLayer(new DemoLayer("DemoLayer"));
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