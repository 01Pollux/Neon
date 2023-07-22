#pragma once

#include <Core/String.hpp>

namespace Neon::Logger
{
    enum class LogSeverity : uint8_t
    {
        Disabled,
        Trace,
        Info,
        Warning,
        Error,
        Fatal,

        Count
    };

    /// <summary>
    /// Initialize the logger.
    /// </summary>
    void Initialize();

    /// <summary>
    /// Shutdown the logger.
    /// </summary>
    void Shutdown();

    /// <summary>
    /// Flush the log file.
    /// </summary>
    void Flush();

    /// <summary>
    /// Test if a log severity should be logged.
    /// </summary>
    [[nodiscard]] bool ShouldLog(
        LogSeverity Severity);

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

#define NEON_LOG_X_TAG(Type, Tag, ...)                    \
    do                                                    \
    {                                                     \
        if (Neon::Logger::ShouldLog(Type))                \
        {                                                 \
            Neon::Logger::LogTag(Type, Tag, __VA_ARGS__); \
        }                                                 \
    } while (false)

#define NEON_LOG_TAG(Type, ...)                   \
    do                                            \
    {                                             \
        if (Neon::Logger::ShouldLog(Type))        \
        {                                         \
            Neon::Logger::Log(Type, __VA_ARGS__); \
        }                                         \
    } while (false)

#define NEON_WARNING_TAG(Tag, ...) NEON_LOG_X_TAG(Neon::Logger::LogSeverity::Warning, Tag, __VA_ARGS__)
#define NEON_ERROR_TAG(Tag, ...)   NEON_LOG_X_TAG(Neon::Logger::LogSeverity::Error, Tag, __VA_ARGS__)
#define NEON_FATAL_TAG(Tag, ...)   NEON_LOG_X_TAG(Neon::Logger::LogSeverity::Fatal, Tag, __VA_ARGS__)

#define NEON_WARNING(...) NEON_LOG_TAG(Neon::Logger::LogSeverity::Warning, __VA_ARGS__)
#define NEON_ERROR(...)   NEON_LOG_TAG(Neon::Logger::LogSeverity::Error, __VA_ARGS__)
#define NEON_FATAL(...)   NEON_LOG_TAG(Neon::Logger::LogSeverity::Fatal, __VA_ARGS__)

#ifndef NEON_DIST

#ifdef NEON_DEBUG
#define NEON_TRACE_TAG(Tag, ...) NEON_LOG_X_TAG(Neon::Logger::LogSeverity::Trace, Tag, __VA_ARGS__)
#define NEON_TRACE(...)          NEON_LOG_TAG(Neon::Logger::LogSeverity::Trace, __VA_ARGS__)
#else
#define NEON_TRACE_TAG(Tag, ...)
#define NEON_TRACE(...)
#endif

#define NEON_INFO_TAG(Tag, ...) NEON_LOG_X_TAG(Neon::Logger::LogSeverity::Info, Tag, __VA_ARGS__)
#define NEON_INFO(...)          NEON_LOG_TAG(Neon::Logger::LogSeverity::Info, __VA_ARGS__)

#define NEON_ASSERT(Expr, ...)                        \
    do                                                \
    {                                                 \
        if (!(Expr)) [[unlikely]]                     \
        {                                             \
            NEON_FATAL_TAG("Assertion", __VA_ARGS__); \
            __debugbreak();                           \
        }                                             \
    } while (false)

#else

#define NEON_TRACE_TAG(Tag, ...)
#define NEON_INFO_TAG(Tag, ...)

#define NEON_TRACE(...)
#define NEON_INFO(...)

#define NEON_ASSERT(Expr, ...) (void)(Expr)

#endif

#define NEON_VALIDATE(Expr, ...)                       \
    do                                                 \
    {                                                  \
        if (!(Expr)) [[unlikely]]                      \
        {                                              \
            NEON_FATAL_TAG("Validation", __VA_ARGS__); \
            std::unreachable();                        \
        }                                              \
    } while (false)

#endif