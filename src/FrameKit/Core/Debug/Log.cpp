// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Debug/Log.cpp
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Implements FrameKit Logger and Log.
// =============================================================================

#include "FrameKit/Engine/PlatformDetection.h"
#include "FrameKit/Debug/Log.h"

#if defined(FK_PLATFORM_WINDOWS)
    #define NOMINMAX
    #include <windows.h>
#endif

#include <cstdio>
#include <cstring>
#include <ctime>

namespace FrameKit {

    std::mutex  Log::s_Mutex;
    Ref<Logger> Log::s_CoreLogger;
    Ref<Logger> Log::s_ClientLogger;

    void Logger::set_file(const std::string& path, bool append) {
        std::scoped_lock lk(m_Mutex);
        m_File.close();
        std::ios_base::openmode mode = std::ios::out;
        if (append) mode |= std::ios::app;
        m_File.open(path, mode);
    }

    std::string Logger::now_hms() noexcept {
        using namespace std::chrono;
        auto t = system_clock::to_time_t(system_clock::now());
        std::tm bt{};
#if defined(FK_PLATFORM_WINDOWS)
        localtime_s(&bt, &t);
#else
        localtime_r(&t, &bt);
#endif
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", bt.tm_hour, bt.tm_min, bt.tm_sec);
        return std::string(buf);
    }

    void Logger::write_line(LogLevel lvl, const std::string& message) noexcept {
        std::ostringstream line;
        line << "[" << now_hms() << "] "
            << "[" << detail::level_to_cstr(lvl) << "] "
            << m_Name << ": " << message;

        const std::string s = line.str();

        std::scoped_lock lk(m_Mutex);
        if (m_Console.load(std::memory_order_relaxed)) {
#if defined(FK_PLATFORM_WINDOWS)
            // Enable ANSI VT once
            static bool vtEnabled = false;
            if (!vtEnabled) {
                if (HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); hOut != INVALID_HANDLE_VALUE) {
                    DWORD mode = 0;
                    if (GetConsoleMode(hOut, &mode)) {
                        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                        SetConsoleMode(hOut, mode);
                    }
                }
                vtEnabled = true;
            }
#endif
            const char* color = detail::level_to_ansi(lvl);
            std::fwrite(color, 1, std::strlen(color), stdout);
            std::fwrite(s.c_str(), 1, s.size(), stdout);
            std::fwrite("\x1b[0m\n", 1, 5, stdout);
            std::fflush(stdout);
        }
        if (m_File.is_open()) {
            m_File << s << '\n';
            m_File.flush();
        }
    }

    // Return a disabled logger so macros are safe pre-init or post-uninit
    Ref<Logger>& Log::Noop() {
        static Ref<Logger> s = CreateRef<Logger>("Noop");
        static std::once_flag once;
        std::call_once(once, [] {
            s->enable_console(false);
            s->set_level(LogLevel::Off);
            });
        return s;
    }

    Ref<Logger>& Log::GetCoreLogger() { return s_CoreLogger ? s_CoreLogger : Noop(); }
    Ref<Logger>& Log::GetClientLogger() { return s_ClientLogger ? s_ClientLogger : Noop(); }

    static void setup_common(Ref<Logger>& logger, const std::string& fileName, LogLevel level) {
        logger->set_level(level);
        logger->enable_console(true);
        if (!fileName.empty()) logger->set_file(fileName, true);
    }

    void Log::Init() {
        std::scoped_lock lk(s_Mutex);
        if (!s_CoreLogger)   s_CoreLogger = CreateRef<Logger>("FrameKit");
        if (!s_ClientLogger) s_ClientLogger = CreateRef<Logger>("Application");
        setup_common(s_CoreLogger, "FrameKit.log", LogLevel::Trace);
        setup_common(s_ClientLogger, "Application.log", LogLevel::Trace);
    }

    void Log::InitClient(std::string name) {
        std::scoped_lock lk(s_Mutex);
        const std::string clientName = name.empty() ? "Application" : std::move(name);

        s_CoreLogger = CreateRef<Logger>("FrameKit");
        s_ClientLogger = CreateRef<Logger>(clientName);

        setup_common(s_CoreLogger, "FrameKit.log", LogLevel::Trace);
        setup_common(s_ClientLogger, clientName + ".log", LogLevel::Trace);
    }

} // namespace FrameKit
