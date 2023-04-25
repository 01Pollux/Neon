#pragma once

#include <Core/String.hpp>

namespace Neon::Logger
{
    enum class LogSeverity
    {
        Info,
        Message,
        Warning,
        Assert,
        Error,
        Fatal
    };

    /// <summary>
    /// Must be called from the main thread, to initialize the logger.
    /// </summary>
    void Initialize();

    /// <summary>
    /// Must be called from the main thread, to shutdown the logger.
    /// </summary>
    void Shutdown();

    void LogData(
        const StringU8& Message,
        LogSeverity     Severity,
        const char*     FunctionName,
        const char*     SourcePath,
        uint32_t        LineNum);
} // namespace Neon::Logger

#if !NEON_DIST

#define NEON_INFO(Msg)                                                                       \
    do                                                                                       \
    {                                                                                        \
        Logger::LogData((Msg), Logger::LogSeverity::Info, __FUNCTION__, __FILE__, __LINE__); \
    } while (false)

#define NEON_MESSAGE(Msg)                                                                     \
    do                                                                                        \
    {                                                                                         \
        Logger::LogData(Msg, Logger::LogSeverity::Message, __FUNCTION__, __FILE__, __LINE__); \
    } while (false)

#define NEON_WARNING(Msg)                                                                     \
    do                                                                                        \
    {                                                                                         \
        Logger::LogData(Msg, Logger::LogSeverity::Warning, __FUNCTION__, __FILE__, __LINE__); \
    } while (false)

#define NEON_ASSERT_MSG(Expr, Msg)                                                               \
    do                                                                                           \
    {                                                                                            \
        if (!(Expr))                                                                             \
        {                                                                                        \
            Logger::LogData(Msg, Logger::LogSeverity::Assert, __FUNCTION__, __FILE__, __LINE__); \
        }                                                                                        \
    } while (false)

#define NEON_ASSERT_RESOURCE_MSG(Expr, ResourceId, ObjectId, ExpectedClass)                         \
    NEON_ASSERT_MSG(                                                                                \
        Expr,                                                                                       \
        StringUtils::Format(                                                                        \
            "Tried to load an invalid resource: {} of Type '{}', Excepted '"## #ExpectedClass##"'", \
            ResourceId,                                                                             \
            ObjectId));

#define NEON_ASSERT(Expr) NEON_ASSERT_MSG(Expr, "!("## #Expr##")")

#define NEON_ERROR(Msg)                                                                     \
    do                                                                                      \
    {                                                                                       \
        Logger::LogData(Msg, Logger::LogSeverity::Error, __FUNCTION__, __FILE__, __LINE__); \
    } while (false)

#define NEON_FATAL(Msg)                                                                     \
    do                                                                                      \
    {                                                                                       \
        const auto& _Msg = Msg;                                                             \
        Logger::LogData(Msg, Logger::LogSeverity::Fatal, __FUNCTION__, __FILE__, __LINE__); \
        throw std::runtime_error(_Msg);                                                     \
    } while (false)

#else

#define NEON_INFO(Msg)
#define NEON_MESSAGE(Msg)
#define NEON_WARNING(Msg)
#define NEON_ASSERT_RESOURCE_MSG(Expr, ...) (void)(Expr)
#define NEON_ASSERT_MSG(Expr, Msg)          (void)(Expr)
#define NEON_ASSERT(Expr)                   (void)(Expr)

#define NEON_ERROR(Msg)                                              \
    do                                                               \
    {                                                                \
        Logger::LogData(Msg, Logger::LogSeverity::Error, "", "", 0); \
    } while (false)

#define NEON_FATAL(Msg)                                              \
    do                                                               \
    {                                                                \
        const auto& _Msg = Msg;                                      \
        Logger::LogData(Msg, Logger::LogSeverity::Fatal, "", "", 0); \
        throw std::runtime_error(_Msg);                              \
    } while (false)

#endif
