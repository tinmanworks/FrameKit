
#include "FrameKit/Application/Application.h"

#include <iostream>

namespace SandBox {

    class SandBoxApp : public FrameKit::Application {
    public:
        SandBoxApp() = default;
        ~SandBoxApp() override = default;

        bool Init() override {
            // Initialization code here
            return true;
        }

        void Shutdown() override {
            // Cleanup code here
        }

        bool OnUpdate(double deltaTime) override {
            // Main update loop code here
            printf("OnUpdate::DeltaTime -> ");
			printf("%f\n", deltaTime);

			return true; // Return false to close the application
        }

        void OnEvent(FrameKit::Event& e) override {
            // Event handling code here
        }
    };

}

FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args) {
    return new SandBox::SandBoxApp();
}