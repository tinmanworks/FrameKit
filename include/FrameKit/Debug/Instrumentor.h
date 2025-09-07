// =============================================================================
// Project      : FrameKit
// File         : include/Debug/Instrumentor.h
// Author       : George Gil
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Defines a simple instrumentation system for profiling C++ code.
// =============================================================================

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <fstream>   // member by value; keep in header to avoid ptr indirection
#include <filesystem>

namespace FrameKit {

using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

struct ProfileResult {
    std::string Name;
    FloatingPointMicroseconds Start;
    std::chrono::microseconds ElapsedTime;
    std::uint64_t ThreadID;
};

struct InstrumentationSession { std::string Name; };

class Instrumentor {
public:
    Instrumentor(const Instrumentor&) = delete;
    Instrumentor& operator=(const Instrumentor&) = delete;
    Instrumentor(Instrumentor&&) = delete;
    Instrumentor& operator=(Instrumentor&&) = delete;

    void BeginSession(const std::string& name,
                      const std::filesystem::path& filepath = "results.json");
    void EndSession();
    void WriteProfile(const ProfileResult& result);

    static Instrumentor& Get() noexcept;

private:
    Instrumentor() = default;
    ~Instrumentor();  // defined in .cpp

    void WriteHeader();
    void WriteFooter();
    void InternalEndSession();           // pre: mutex held
    static std::string EscapeForJson(const std::string& s);

private:
    std::mutex m_Mutex;
    std::unique_ptr<InstrumentationSession> m_CurrentSession;
    std::ofstream m_OutputStream;
};

class InstrumentationTimer {
public:
    explicit InstrumentationTimer(const char* name) noexcept;
    ~InstrumentationTimer();

    void Stop() noexcept;

private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    bool m_Stopped;
};

namespace InstrumentorUtils {
    template <std::size_t N>
    struct ChangeResult { char Data[N]{}; };

    template <std::size_t N, std::size_t K>
    [[nodiscard]] constexpr auto CleanupOutputString(const char(&expr)[N],
                                                     const char(&remove)[K]) {
        ChangeResult<N> result{};
        std::size_t srcIndex = 0, dstIndex = 0;
        while (srcIndex < N) {
            std::size_t matchIndex = 0;
            while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 &&
                   expr[srcIndex + matchIndex] == remove[matchIndex]) ++matchIndex;
            if (matchIndex == K - 1) srcIndex += matchIndex;
            result.Data[dstIndex++] = (expr[srcIndex] == '\"') ? '\'' : expr[srcIndex];
            ++srcIndex;
        }
        return result;
    }

    template <std::size_t N>
    [[nodiscard]] constexpr auto CleanFunctionSig(const char(&expr)[N]) {
        return CleanupOutputString(
                 CleanupOutputString(
                   CleanupOutputString(expr, "__cdecl ").Data, "__stdcall ").Data,
                 "__thiscall ");
    }
} // namespace InstrumentorUtils
} // namespace FrameKit

// ===== Macros =====
#if defined(_MSC_VER)
#  define FK_FUNC_SIG __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__)
#  define FK_FUNC_SIG __PRETTY_FUNCTION__
#else
#  define FK_FUNC_SIG __func__
#endif

#if FK_PROFILE
#  define FK_PROFILE_BEGIN_SESSION(name, filepath) \
       ::FrameKit::Instrumentor::Get().BeginSession(name, filepath)
#  define FK_PROFILE_END_SESSION() \
       ::FrameKit::Instrumentor::Get().EndSession()
#  define FK_PROFILE_SCOPE_LINE2(name, line) \
       constexpr auto fixedName##line = ::FrameKit::InstrumentorUtils::CleanFunctionSig(name); \
       ::FrameKit::InstrumentationTimer timer##line(fixedName##line.Data)
#  define FK_PROFILE_SCOPE_LINE(name, line) FK_PROFILE_SCOPE_LINE2(name, line)
#  define FK_PROFILE_SCOPE(name)            FK_PROFILE_SCOPE_LINE(name, __LINE__)
#  define FK_PROFILE_FUNCTION()             FK_PROFILE_SCOPE(FK_FUNC_SIG)
#else
#  define FK_PROFILE_BEGIN_SESSION(name, filepath)
#  define FK_PROFILE_END_SESSION()
#  define FK_PROFILE_SCOPE(name)
#  define FK_PROFILE_FUNCTION()
#endif
