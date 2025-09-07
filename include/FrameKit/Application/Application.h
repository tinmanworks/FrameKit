// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Application/Application.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines the Application class for the FrameKit framework. Inherits from
//      ApplicationBase. Applications should derive from this to implement their
//      own Init/Shutdown and custom logic.
// =============================================================================

#pragma once

#include "FrameKit/Base/ApplicationBase.h"
#include "FrameKit/Events/ApplicationEvent.h"

namespace FrameKit {

    class Application : public ApplicationBase {
    public:
        Application();
        explicit Application(const ApplicationSpecification& spec);
        ~Application() override;

        // User overridable lifecycle hooks
        void Init() override;
        void Shutdown() override;
        void PreRunloopIteration() override;
        void PostRunloopIteration() override;
        void OnEvent(Event& e) override;

        void Close();

        static Application& Get() { return *s_Instance; }

    protected:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        float m_LastFrameTime = 0.0f;
        bool  m_Minimized     = false;

        static Application* s_Instance;
    };

    // To be defined by CLIENT
    Application* CreateApplication(ApplicationCommandLineArgs args);

} // namespace FrameKit
