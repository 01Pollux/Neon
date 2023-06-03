#pragma once

#include <Core/String.hpp>

namespace Neon::Logger
{
    enum class LogSeverity : uint8_t
    {
        Disabled,
        Info,
        Trace,
        Warning,
        Error,
        Fatal,

        Count
    };

    /// <summary>
    /// Must be called from the main thread, to initialize the logger.
    /// </summary>
    void Initialize();

    /// <summary>
    /// Must be called from the main thread, to shutdown the logger.
    /// </summary>
    void Shutdown();

    /// <summary>
    /// Enable or disable logging and the severity of the logs.
    /// </summary>
    void SetLogTag(
        const StringU8& Tag,
        LogSeverity     Severity,
        bool            Enabled = true);

    /// <summary>
    /// Log a message to the log file or console.
    /// </summary>
    void LogMessage(
        LogSeverity     Severity,
        const StringU8& Tag,
        const StringU8& Message);

    /// <summary>
    /// Log a message to the log file or console.
    /// </summary>
    void LogMessage(
        LogSeverity     Severity,
        const StringU8& Message);

    /// <summary>
    /// Log a message to the log file or console.
    /// </summary>
    template<typename... _Args>
    void LogTag(
        LogSeverity     Severity,
        const StringU8& Tag,
        const StringU8& Message = "",
        _Args&&... Args)
    {
        LogMessage(Severity, Tag, StringUtils::Format(Message, std::forward<_Args>(Args)...));
    }
    /// <summary>
    /// Log a message to the log file or console.
    /// </summary>
    template<typename... _Args>
    void Log(
        LogSeverity     Severity,
        const StringU8& Message = "",
        _Args&&... Args)
    {
        LogMessage(Severity, StringUtils::Format(Message, std::forward<_Args>(Args)...));
    }
} // namespace Neon::Logger

#ifdef NEON_NO_LOG

#define NEON_WARNING_TAG(Tag, ...)
#define NEON_ERROR_TAG(Tag, ...)
#define NEON_FATAL_TAG(Tag, ...)

#define NEON_WARNING(...)
#define NEON_ERROR(...)
#define NEON_FATAL(...)

#define NEON_TRACE_TAG(Tag, ...)
#define NEON_INFO_TAG(Tag, ...)

#define NEON_TRACE(...) NEON_TRACE_TAG("", __VA_ARGS__)
#define NEON_INFO(...)  NEON_INFO_TAG("", __VA_ARGS__)

#define NEON_ASSERT(Expr, ...)   (void)(Expr)
#define NEON_VALIDATE(Expr, ...) (void)(Expr)

#else

#define NEON_WARNING_TAG(Tag, ...) Neon::Logger::LogTag(Neon::Logger::LogSeverity::Warning, Tag, __VA_ARGS__)
#define NEON_ERROR_TAG(Tag, ...)   Neon::Logger::LogTag(Neon::Logger::LogSeverity::Error, Tag, __VA_ARGS__)
#define NEON_FATAL_TAG(Tag, ...)   Neon::Logger::LogTag(Neon::Logger::LogSeverity::Fatal, Tag, __VA_ARGS__)

#define NEON_WARNING(...) Neon::Logger::Log(Neon::Logger::LogSeverity::Warning, __VA_ARGS__)
#define NEON_ERROR(...)   Neon::Logger::Log(Neon::Logger::LogSeverity::Error, __VA_ARGS__)
#define NEON_FATAL(...)   Neon::Logger::Log(Neon::Logger::LogSeverity::Fatal, __VA_ARGS__)

#ifndef NEON_DIST

#ifdef NEON_DEBUG
#define NEON_TRACE_TAG(Tag, ...) Neon::Logger::LogTag(Neon::Logger::LogSeverity::Trace, Tag, __VA_ARGS__)
#else
#define NEON_TRACE_TAG(Tag, ...)
#endif

#define NEON_INFO_TAG(Tag, ...) Neon::Logger::LogTag(Neon::Logger::LogSeverity::Trace, Tag, __VA_ARGS__)

#define NEON_TRACE(...) Neon::Logger::Log(Neon::Logger::LogSeverity::Trace, __VA_ARGS__)
#define NEON_INFO(...)  Neon::Logger::Log(Neon::Logger::LogSeverity::Info, __VA_ARGS__)

#define NEON_ASSERT(Expr, ...)                                \
    do                                                        \
    {                                                         \
        if (!(Expr))                                          \
        {                                                     \
            NEON_FATAL_TAG("Assertion Failure", __VA_ARGS__); \
            __debugbreak();                                   \
        }                                                     \
    } while (false)

#else

#define NEON_TRACE_TAG(Tag, ...)
#define NEON_INFO_TAG(Tag, ...)

#define NEON_TRACE(...) NEON_TRACE_TAG("", __VA_ARGS__)
#define NEON_INFO(...)  NEON_INFO_TAG("", __VA_ARGS__)

#define NEON_ASSERT(Expr, ...) (void)(Expr)

#endif

#define NEON_VALIDATE(Expr, ...)                               \
    do                                                         \
    {                                                          \
        if (!(Expr))                                           \
        {                                                      \
            NEON_FATAL_TAG("Validation Failure", __VA_ARGS__); \
        }                                                      \
    } while (false)

#endif