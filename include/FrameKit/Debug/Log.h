// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Debug/Log.h 
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-09
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Thread-safe logging with console/file sinks, levels, and '{}' formatting.
// =============================================================================

#pragma once

#include "FrameKit/Engine/Defines.h"
#include "FrameKit/Utilities/Memory.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace FrameKit {

    enum class LogLevel : std::uint8_t { Trace, Info, Warn, Error, Critical, Off };

    namespace detail {
        template<typename... Args>
        inline std::string vformat(std::string_view fmt, Args&&... args) {
            std::ostringstream oss;
            std::ostringstream tmp;
            std::vector<std::string> vals;
            vals.reserve(sizeof...(Args));
            ((tmp.str(""), tmp.clear(), tmp << std::forward<Args>(args), vals.push_back(tmp.str())), ...);

            size_t i = 0, n = fmt.size(), argi = 0;
            while (i < n) {
                if (fmt[i] == '{' && i + 1 < n && fmt[i + 1] == '}') {
                    oss << (argi < vals.size() ? vals[argi++] : "{}");
                    i += 2;
                }
                else if (fmt[i] == '{' && i + 1 < n && fmt[i + 1] == '{') {
                    oss << '{'; i += 2;
                }
                else if (fmt[i] == '}' && i + 1 < n && fmt[i + 1] == '}') {
                    oss << '}'; i += 2;
                }
                else {
                    oss << fmt[i++];
                }
            }
            return oss.str();
        }

        inline const char* level_to_cstr(LogLevel lvl) {
            switch (lvl) {
            case LogLevel::Trace:    return "TRACE";
            case LogLevel::Info:     return "INFO";
            case LogLevel::Warn:     return "WARN";
            case LogLevel::Error:    return "ERROR";
            case LogLevel::Critical: return "CRITICAL";
            default:                 return "OFF";
            }
        }

        inline const char* level_to_ansi(LogLevel lvl) {
            switch (lvl) {
            case LogLevel::Trace:    return "\x1b[90m";
            case LogLevel::Info:     return "\x1b[97m";
            case LogLevel::Warn:     return "\x1b[93m";
            case LogLevel::Error:    return "\x1b[91m";
            case LogLevel::Critical: return "\x1b[95m";
            default:                 return "\x1b[0m";
            }
        }
    } // namespace detail

    class Logger {
    public:
        explicit Logger(std::string name) : m_Name(std::move(name)) {}

        void set_level(LogLevel lvl) { m_Level.store(lvl, std::memory_order_relaxed); }
        LogLevel level() const { return m_Level.load(std::memory_order_relaxed); }

        void set_file(const std::string& path, bool append = true);
        void enable_console(bool on) { m_Console.store(on, std::memory_order_relaxed); }

        template<typename... Args>
        void log(LogLevel lvl, std::string_view fmt, Args&&... args) noexcept {
            const auto cur = m_Level.load(std::memory_order_relaxed);
            if (cur == LogLevel::Off || lvl < cur) return;
            const auto msg = detail::vformat(fmt, std::forward<Args>(args)...);
            write_line(lvl, msg);
        }

        template<typename... Args> void trace(std::string_view f, Args&&... a) { log(LogLevel::Trace, f, std::forward<Args>(a)...); }
        template<typename... Args> void info(std::string_view f, Args&&... a) { log(LogLevel::Info, f, std::forward<Args>(a)...); }
        template<typename... Args> void warn(std::string_view f, Args&&... a) { log(LogLevel::Warn, f, std::forward<Args>(a)...); }
        template<typename... Args> void error(std::string_view f, Args&&... a) { log(LogLevel::Error, f, std::forward<Args>(a)...); }
        template<typename... Args> void critical(std::string_view f, Args&&... a) { log(LogLevel::Critical, f, std::forward<Args>(a)...); }

    private:
        static std::string now_hms() noexcept;
        void write_line(LogLevel lvl, const std::string& message) noexcept;

        std::string              m_Name;
        std::atomic<LogLevel>    m_Level{ LogLevel::Trace };
        std::atomic<bool>        m_Console{ true };
        std::ofstream            m_File;
        std::mutex               m_Mutex;
    };

    class Log {
    public:
        // Initializes default "FrameKit" core and "Application" client.
        static void Init();
        // Initializes client with a custom name. Core remains "FrameKit".
        static void InitClient(std::string name);
        static void UninitClient();

        // Safe getters: return a valid no-op logger if not initialized.
        static Ref<Logger>& GetCoreLogger();
        static Ref<Logger>& GetClientLogger();

    private:
        static Ref<Logger>& Noop();

        static std::mutex  s_Mutex;
        static Ref<Logger> s_CoreLogger;
        static Ref<Logger> s_ClientLogger;
    };

} // namespace FrameKit

// Core log macros
#define FK_CORE_TRACE(...)    ::FrameKit::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define FK_CORE_INFO(...)     ::FrameKit::Log::GetCoreLogger()->info(__VA_ARGS__)
#define FK_CORE_WARN(...)     ::FrameKit::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define FK_CORE_ERROR(...)    ::FrameKit::Log::GetCoreLogger()->error(__VA_ARGS__)
#define FK_CORE_CRITICAL(...) ::FrameKit::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define FK_TRACE(...)         ::FrameKit::Log::GetClientLogger()->trace(__VA_ARGS__)
#define FK_INFO(...)          ::FrameKit::Log::GetClientLogger()->info(__VA_ARGS__)
#define FK_WARN(...)          ::FrameKit::Log::GetClientLogger()->warn(__VA_ARGS__)
#define FK_ERROR(...)         ::FrameKit::Log::GetClientLogger()->error(__VA_ARGS__)
#define FK_CRITICAL(...)      ::FrameKit::Log::GetClientLogger()->critical(__VA_ARGS__)
