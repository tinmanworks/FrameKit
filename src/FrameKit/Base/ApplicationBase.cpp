// =============================================================================
// Project      : FrameKit
// File         : ApplicationBase.cpp
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Implements the ApplicationBase class for the FrameKit framework.
//      Provides lifecycle management, threading, event dispatch, and
//      layer management functionality for derived applications.
// =============================================================================

#include "FrameKit/Base/ApplicationBase.h"
#include "FrameKit/InterProcess/InterprocessEventManager.h"
#include "FrameKit/Events/GlobalEventHandler.h"

#include <chrono>
#include <filesystem>
using namespace std::chrono_literals;

namespace FrameKit {

    ApplicationBase::ApplicationBase(const ApplicationSpecification& specification)
        : m_Specification(specification)
    {
        m_Master = m_Specification.Master;

        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);

        if (!m_Master) {
            using namespace IPC;
            const uint32_t key = GenerateKey("CloseBase");
            m_InterProcManager.RegisterCallback(key, [this]() {
                SubmitToMainThread([this]{ CloseBase(); });
            });
        }

        GlobalEventHandler::Get().SetEventCallback(FK_BIND_EVENT_FN(ApplicationBase::OnEventBase));
    }

    ApplicationBase::~ApplicationBase() {
        m_Running.store(false, std::memory_order_release);
        if (m_ThreadCyclic && m_ThreadCyclic->joinable())
            m_ThreadCyclic->join();
    }

    void ApplicationBase::StartAsyncThread() {
        if (m_ThreadCyclic && m_ThreadCyclic->joinable())
            return;
        m_Running.store(true, std::memory_order_release);
        m_ThreadCyclic = CreateRef<std::thread>([this]{ ThreadCyclic(); });
    }

    void ApplicationBase::StopAsyncThread() {
        m_Running.store(false, std::memory_order_release);
        if (m_ThreadCyclic && m_ThreadCyclic->joinable())
            m_ThreadCyclic->join();
        m_ThreadCyclic.reset();
    }

    void ApplicationBase::ThreadCyclic() {
        while (m_Running.load(std::memory_order_acquire)) {
            if (!m_Master) m_InterProcManager.CheckSharedMemory();
            {
                std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
                for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
                    if (*it) (*it)->OnCyclic();
                }
            }
            std::this_thread::sleep_for(5ms);
        }
    }

    void ApplicationBase::PushLayer(Layer* layer) {
        std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
        m_LayerStack->PushLayer(layer);
        layer->OnAttach();
    }

    void ApplicationBase::PushOverlay(Layer* layer) {
        std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
        m_LayerStack->PushOverlay(layer);
        layer->OnAttach();
    }

    void ApplicationBase::SubmitToMainThread(const std::function<void()>& fn) {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
        m_MainThreadQueue.emplace_back(fn);
    }

    void ApplicationBase::SubmitToMainThread(std::function<void()>&& fn) {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
        m_MainThreadQueue.emplace_back(std::move(fn));
    }

    void ApplicationBase::SendEventToSlave(const IPC::InterprocessEvent& e) {
        m_InterProcManager.PushDescriptor(e);
    }

    void ApplicationBase::RunBase() {
        while (m_Running.load(std::memory_order_acquire)) {
            ExecuteMainThreadQueue();
            if (!m_Running.load(std::memory_order_acquire))
                break;
            PreRunloopIteration();
            RunLoopIteration();
            PostRunloopIteration();
            if (m_Window) m_Window->OnUpdate();
        }
    }

    void ApplicationBase::CloseBase() {
        if (m_Master) {
            IPC::InterprocessEvent e{"CloseBase"};
            SendEventToSlave(e);
        }
        m_Running.store(false, std::memory_order_release);
        Shutdown();
    }

    void ApplicationBase::ExecuteMainThreadQueue() {
        std::vector<std::function<void()>> work;
        {
            std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
            work.swap(m_MainThreadQueue);
        }
        for (auto& fn : work) fn();
    }

    void ApplicationBase::OnEventBase(Event& e) {
        {
            std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
            for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
                if (e.Handled) break;
                (*it)->OnEvent(e);
            }
        }
        if (!e.Handled) OnEvent(e);
    }

} // namespace FrameKit
