// =============================================================================
// Project      : FrameKit
// File         : Application.cpp
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Implements the Application class for the FrameKit framework.
// =============================================================================

#include "FrameKit/Application/Application.h"
#include "FrameKit/Debug/Log.h"
// #include "FrameKit/Window/Window.h"

namespace FrameKit {

    Application* Application::s_Instance = nullptr;

    Application::Application()
    : Application(ApplicationSpecification{}) {}

    Application::Application(const ApplicationSpecification& spec)
    : ApplicationBase(spec)
    {
        FK_CORE_ASSERT(!s_Instance, "Application already exists");
        s_Instance = this;

        if (m_Specification.Windowed) {
            auto win = Window::Create(WindowProps(m_Specification.Name));
            win->SetEventCallback([this](Event& e){ OnEvent(e); });
            SetWindow(std::move(win));
        }
    }

    Application::~Application() {
        s_Instance = nullptr;
    }

    void Application::Init() {
        m_LastFrameTime = Time::GetTime();
    }

    void Application::Shutdown() {
        // user cleanup point
    }

    void Application::PreRunloopIteration() {
        // user pre-frame work
    }

    void Application::PostRunloopIteration() {
        // user post-frame work
    }

    void Application::OnEvent(Event& e) {
        if (!m_Specification.Windowed) return;
        EventDispatcher d(e);
        d.Dispatch<WindowCloseEvent>(FK_BIND_EVENT_FN(Application::OnWindowClose));
        d.Dispatch<WindowResizeEvent>(FK_BIND_EVENT_FN(Application::OnWindowResize));
    }

    bool Application::OnWindowClose(WindowCloseEvent&) {
        SubmitToMainThread([this]{ Close(); });
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e) {
        m_Minimized = (e.GetWidth()==0 || e.GetHeight()==0);
        return false;
    }

    void Application::Close() {
        CloseBase();
    }

} // namespace FrameKit
