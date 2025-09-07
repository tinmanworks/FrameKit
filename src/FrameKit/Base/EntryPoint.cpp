// =============================================================================
// Project: FrameKit
// File         : EntryPoint.cpp
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//   Engine-owned entry point. Calls client-provided CreateApplication()
//   to obtain the app instance and runs its lifecycle. Modernized with RAII,
//   exception safety, and platform-guarded crash handling.
// =============================================================================

#include "FrameKit/Base/Base.h"
#include "FrameKit/Debug/Instrumentor.h"
#include "FrameKit/Application/Application.h"

#include <exception>
#include <memory>
#include <iostream>

#if defined(FK_PLATFORM_WINDOWS)
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <timeapi.h>
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "winmm.lib")
#elif defined(FK_PLATFORM_LINUX)
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#endif

// Implemented by the executable project:
extern FrameKit::Application* FrameKit::CreateApplication(FrameKit::ApplicationCommandLineArgs args);

namespace {

// -------- RAII helpers for profiling --------
struct ProfileSession {
    ProfileSession(const char* name, const char* path) {
        FK_PROFILE_BEGIN_SESSION(name, path);
    }
    ~ProfileSession() {
        FK_PROFILE_END_SESSION();
    }
};

// -------- Platform helpers --------
#if defined(FK_PLATFORM_WINDOWS)

struct TimePeriod {
    explicit TimePeriod(UINT ms) : value(ms) { timeBeginPeriod(value); }
    ~TimePeriod() { timeEndPeriod(value); }
    UINT value;
};

struct ExceptionFilter {
    ExceptionFilter() : prev_(SetUnhandledExceptionFilter(&ExceptionHandler)) {}
    ~ExceptionFilter() { SetUnhandledExceptionFilter(prev_); }

    static LONG WINAPI ExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
        // TODO: consider logging the exception code and address
        // TODO: consider making the dump path configurable
        HANDLE hFile = CreateFileW(L"FrameKit_Crashdump.dmp",
            GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != nullptr && hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION mdei{};
            mdei.ThreadId = GetCurrentThreadId();
            mdei.ExceptionPointers = pExceptionInfo;
            mdei.ClientPointers = FALSE;
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                MiniDumpNormal, &mdei, nullptr, nullptr);
            CloseHandle(hFile);
        }
        return EXCEPTION_EXECUTE_HANDLER;
    }

private:
    LPTOP_LEVEL_EXCEPTION_FILTER prev_;
};

#elif defined(FK_PLATFORM_LINUX)

// Minimal Linux crash backtrace (optional)
struct SignalHandler {
    SignalHandler() {
        struct sigaction sa {};
        sa.sa_sigaction = &SignalHandler::Handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO | SA_RESTART;
        sigaction(SIGSEGV, &sa, &old_segv_);
        sigaction(SIGABRT, &sa, &old_abrt_);
        sigaction(SIGFPE,  &sa, &old_fpe_);
        sigaction(SIGILL,  &sa, &old_ill_);
    }
    ~SignalHandler() {
        sigaction(SIGSEGV, &old_segv_, nullptr);
        sigaction(SIGABRT, &old_abrt_, nullptr);
        sigaction(SIGFPE,  &old_fpe_,  nullptr);
        sigaction(SIGILL,  &old_ill_,  nullptr);
    }

    static void Handler(int sig, siginfo_t*, void*) {
        void* array[64];
        int size = backtrace(array, 64);
        fprintf(stderr, "FrameKit crashed with signal %d\nBacktrace:\n", sig);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        _exit(128 + sig);
    }

private:
    struct sigaction old_segv_{}, old_abrt_{}, old_fpe_{}, old_ill_{};
};

#endif // FK_PLATFORM_LINUX

} // namespace

int main(int argc, char** argv) {
#if defined(FK_PLATFORM_WINDOWS)
    TimePeriod tp(1);       // Raise timer resolution while the app runs
    ExceptionFilter filter; // Write a minidump on unhandled exceptions
#elif defined(FK_PLATFORM_LINUX)
    SignalHandler sigs;     // Print a backtrace on common fatal signals
#endif

    FrameKit::Log::Init();

    try {
        // Startup session: create and initialize the app
        std::unique_ptr<FrameKit::Application> app;
        {
            // TODO: consider moving Init() out of this scope if it throws
            // TODO: Consider startup profile session name and path from args or dynamically from application name
            ProfileSession startup{ "Startup", "FrameKit-Startup.json" };
            app.reset(FrameKit::CreateApplication({ argc, argv }));
            app->Init();
        }

        // Runtime session: run the app loop
        {
            // TODO: Consider runtime profile session name and path from args or dynamically from application name
            ProfileSession runtime{ "Runtime", "FrameKit-Runtime.json" };
            app->RunBase();
        }

        // Shutdown session: if the loop exited by CloseBase() it already called Shutdown()
        {
            // TODO: Consider shutdown profile session name and path from args or dynamically from application name
            ProfileSession shutdown{ "Shutdown", "FrameKit-Shutdown.json" };
            // If a derived app stopped the loop without calling CloseBase(), call Shutdown() here.
            // This is safe if Shutdown() is idempotent.
            // Optional: guard with an internal flag if you add one.
            app->Shutdown();
        }

    }
    catch (const std::exception& e) {
        FK_CORE_ERROR("Unhandled std::exception in entry: {}", e.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        FK_CORE_ERROR("Unhandled non-standard exception in entry.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}