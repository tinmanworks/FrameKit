// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Debug/Instrumentor.cpp
// Author       : George Gil
// Created      : 2025-09-09
// Updated      : 2025-09-09
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Instrumentation system for profiling C++ code.
// =============================================================================

#include "FrameKit/Debug/Instrumentor.h"
#include "FrameKit/Debug/Log.h"  // used only for error logs

#include <iomanip>
#include <sstream>
#include <thread>

namespace FrameKit {

    Instrumentor& Instrumentor::Get() noexcept {
        static Instrumentor instance;
        return instance;
    }

    Instrumentor::~Instrumentor() { EndSession(); }

    void Instrumentor::BeginSession(const std::string& name,
        const std::filesystem::path& filepath) {
        std::scoped_lock lock(m_Mutex);

        if (m_CurrentSession) {
            if (auto& lg = Log::GetCoreLogger(); lg)
                lg->error("Instrumentor::BeginSession('{}') when session '{}' already open.",
                    name, m_CurrentSession->Name);
            InternalEndSession();
        }

        std::error_code ec;
        const auto parent = filepath.parent_path();
        if (!parent.empty()) std::filesystem::create_directories(parent, ec);

        m_OutputStream.open(filepath, std::ios::out | std::ios::trunc);
        if (m_OutputStream.is_open()) {
            m_CurrentSession = std::make_unique<InstrumentationSession>(InstrumentationSession{ name });
            WriteHeader();
        }
        else {
            if (auto& lg = Log::GetCoreLogger(); lg)
                lg->error("Instrumentor could not open results file '{}'.", filepath.string());
        }
    }

    void Instrumentor::EndSession() {
        std::scoped_lock lock(m_Mutex);
        InternalEndSession();
    }

    void Instrumentor::InternalEndSession() {
        if (m_CurrentSession) {
            WriteFooter();
            m_OutputStream.close();
            m_CurrentSession.reset();
        }
    }

    void Instrumentor::WriteHeader() {
        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
        m_OutputStream.flush();
    }

    void Instrumentor::WriteFooter() {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

    std::string Instrumentor::EscapeForJson(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
            }
        }
        return out;
    }

    void Instrumentor::WriteProfile(const ProfileResult& result) {
        std::ostringstream json;
        json << std::setprecision(3) << std::fixed;
        json << ",{"
            << "\"cat\":\"function\","
            << "\"dur\":" << result.ElapsedTime.count() << ','
            << "\"name\":\"" << EscapeForJson(result.Name) << "\","
            << "\"ph\":\"X\","
            << "\"pid\":0,"
            << "\"tid\":" << result.ThreadID << ","
            << "\"ts\":" << result.Start.count()
            << "}";
        std::scoped_lock lock(m_Mutex);
        if (m_CurrentSession) {
            m_OutputStream << json.str();
            m_OutputStream.flush();
        }
    }

    // -------- Timer --------
    InstrumentationTimer::InstrumentationTimer(const char* name) noexcept
        : m_Name(name),
        m_StartTimepoint(std::chrono::steady_clock::now()),
        m_Stopped(false) {
    }

    InstrumentationTimer::~InstrumentationTimer() {
        if (!m_Stopped) Stop();
    }

    void InstrumentationTimer::Stop() noexcept {
        const auto endTimepoint = std::chrono::steady_clock::now();

        const auto start_us = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();
        const auto end_us = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch();
        const auto elapsed = end_us - start_us;
        const auto start_f64 = FloatingPointMicroseconds{ start_us }; // implicit conversion OK

        const std::uint64_t tid =
            static_cast<std::uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

        Instrumentor::Get().WriteProfile({ m_Name, start_f64, elapsed, tid });
        m_Stopped = true;
    }

} // namespace FrameKit
