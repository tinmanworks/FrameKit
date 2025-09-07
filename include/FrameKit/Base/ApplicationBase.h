// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Base/ApplicationBase.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines the ApplicationBase class and related specifications for the
//      FrameKit framework. Handles lifecycle, threading, event dispatch, and
//      layer management for derived applications.
// =============================================================================

#pragma once

#include "FrameKit/Base/Base.h"
#include "FrameKit/Base/LayerStack.h"

#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

namespace FrameKit {
// ---------- forward decls for minimal coupling ----------
    class Window;

    namespace IPC {
        struct InterprocessEvent;
        class InterprocessEventManager;
    }

    // ---------- CLI args ----------
    struct ApplicationCommandLineArgs {
        int    Count = 0;
        char** Args  = nullptr;
        const char* operator[](int i) const noexcept { return (i>=0 && i<Count) ? Args[i] : nullptr; }
    };

    // ---------- App spec ----------
    struct ApplicationSpecification {
        std::string                Name             = "FrameKit Application";
        std::string                WorkingDirectory = {};
        ApplicationCommandLineArgs CommandLineArgs  = {};
        bool                       Windowed         = false;
        bool                       VSync            = true;
        bool                       Master           = false;
    };

    // ---------- Base application ----------
    class ApplicationBase {
    public:
        ApplicationBase();
        explicit ApplicationBase(const ApplicationSpecification& specification);
        virtual ~ApplicationBase();

        ApplicationBase(const ApplicationBase&)            = delete;
        ApplicationBase& operator=(const ApplicationBase&) = delete;
        ApplicationBase(ApplicationBase&&)                 = delete;
        ApplicationBase& operator=(ApplicationBase&&)      = delete;

        // ---- lifecycle hooks for derived/user application ----
        virtual void Init() = 0;
        virtual void Shutdown() = 0;
        virtual void PreRunloopIteration() = 0;

        virtual void RunLoopIteration() {
            RunLoopRenderProcess();
            LayerStackOnUpdate();
            LayerStackOnPostUpdate();
        }

        virtual void PostRunloopIteration() = 0;
        virtual void OnEvent(Event& event) = 0;

        // ---- run loop driver ----
        void RunBase();

        // ---- threading helpers ----
        void StartAsyncThread();
        void StopAsyncThread();

        // ---- runtime layer management ----
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        // ---- main-thread job queue ----
        void SubmitToMainThread(const std::function<void()>& fn);
        void SubmitToMainThread(std::function<void()>&& fn);

        // --- IPC helpers ---
        void SendEventToSlave(const IPC::InterprocessEvent& e);

        template<typename Callback>
        void RegisterEventCallback(uint32_t key, Callback&& cb) {
            RegisterEventCallbackImpl(key, std::forward<Callback>(cb));
        }

    protected:
        void ExecuteMainThreadQueue();
        void RunLoopRenderProcess();
        void LayerStackOnUpdate();
        void LayerStackOnPostUpdate();

        void              SetWindow(Scope<Window>&& w) { m_Window = std::move(w); }
        Window*           GetWindow()                  { return m_Window.get(); }
        const Window*     GetWindow() const            { return m_Window.get(); }

        void CloseBase();           // stop loop, notify slaves
        void OnEventBase(Event& e); // dispatch through layers then app

        ApplicationSpecification m_Specification{};
        LayerStack*              m_LayerStack;
        std::mutex               m_LayerStackMutex;

        std::atomic<bool> m_Running { true };
        bool              m_Master  = false;

    private:
        Scope<Window>                       m_Window;
        Ref<std::thread>                    m_ThreadCyclic;
        std::mutex                          m_MainThreadQueueMutex;
        std::vector<std::function<void()>>  m_MainThreadQueue;

        bool              m_SharedMemoryAttached = false;
        // TODO: enable IPC later
        // IPC::InterprocessEventManager m_InterProcManager;

        template<typename Callback>
        void RegisterEventCallbackImpl(uint32_t key, Callback&& cb);

        void ThreadCyclic();
    };

} // namespace FrameKit
