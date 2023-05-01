#include <CorePCH.hpp>
#include <Log/Logger.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

#include <filesystem>

namespace Neon::Logger
{
    struct LogDetails
    {
        LogSeverity Severity;
        bool        Enabled;
    };

    static inline std::map<StringU8, LogDetails> s_LogDetails;

    static inline uint32_t   s_LogInitCount = 0;
    static inline std::mutex s_LogInitMutex;

    static inline std::unique_ptr<spdlog::logger> s_EngineLogger;

    void Initialize()
    {
        std::lock_guard LogInit(s_LogInitMutex);
        if (!s_LogInitCount++)
        {
            if (!std::filesystem::exists("Logs"))
                std::filesystem::create_directories("Logs");

            std::vector<spdlog::sink_ptr> Sinks{
                std::make_shared<spdlog::sinks::basic_file_sink_mt>("Logs/Engine.log", true),
#ifndef NEON_DIST
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
                std::make_shared<spdlog::sinks::msvc_sink_mt>()
#endif
            };

            Sinks[0]->set_pattern("[%c] [%l] %n: %v.");

#ifndef NEON_DIST
            for (auto& Skin : Sinks | std::views::drop(1))
            {
                Skin->set_pattern("%^[%T] %n: %v%$.");
            }
#endif

            s_EngineLogger = std::make_unique<spdlog::logger>("Neon", Sinks.begin(), Sinks.end());

#ifdef NEON_DEBUG
            s_EngineLogger->set_level(spdlog::level::trace);
#elif defined NEON_RELEASE
            s_EngineLogger->set_level(spdlog::level::info);
#elif defined NEON_DIST
            s_EngineLogger->set_level(spdlog::level::warn);
#endif
        }
    }

    void LogMessage(
        LogSeverity     Severity,
        const StringU8& Tag,
        const StringU8& Message)
    {
#ifdef NEON_DIST
        if (Severity < Logger::LogSeverity::Warning)
        {
            return;
        }
#endif

        auto& Detail = s_LogDetails[Tag];
        if (Detail.Enabled && Detail.Severity <= Severity)
        {
            switch (Severity)
            {
#ifndef NEON_DIST
            case Logger::LogSeverity::Trace:
                s_EngineLogger->trace("[{0}] {1}", Tag, Message);
                break;
            case Logger::LogSeverity::Info:
                s_EngineLogger->info("[{0}] {1}", Tag, Message);
                break;
#endif
            case Logger::LogSeverity::Warning:
                s_EngineLogger->warn("[{0}] {1}", Tag, Message);
                break;
            case Logger::LogSeverity::Error:
                s_EngineLogger->error("[{0}] {1}", Tag, Message);
                break;
            case Logger::LogSeverity::Fatal:
                s_EngineLogger->critical("[{0}] {1}", Tag, Message);
                break;
            }
        }
    }

    void LogMessage(
        LogSeverity     Severity,
        const StringU8& Message)
    {
#ifdef NEON_DIST
        if (Severity < Logger::LogSeverity::Warning)
        {
            return;
        }
#endif

        auto& Detail = s_LogDetails[""];
        if (Detail.Enabled && Detail.Severity <= Severity)
        {
            switch (Severity)
            {
#ifndef NEON_DIST
            case Logger::LogSeverity::Trace:
                s_EngineLogger->trace(Message);
                break;
            case Logger::LogSeverity::Info:
                s_EngineLogger->info(Message);
                break;
#endif
            case Logger::LogSeverity::Warning:
                s_EngineLogger->warn(Message);
                break;
            case Logger::LogSeverity::Error:
                s_EngineLogger->error(Message);
                break;
            case Logger::LogSeverity::Fatal:
                s_EngineLogger->critical(Message);
                break;
            }
        }
    }

    void Shutdown()
    {
        std::lock_guard LogInit(s_LogInitMutex);
        if (!--s_LogInitCount)
        {
            s_EngineLogger = nullptr;
            spdlog::drop_all();
        }
    }

    void SetLogTag(
        const StringU8& Tag,
        LogSeverity     Severity,
        bool            Enabled)
    {
        auto& Detail    = s_LogDetails[Tag];
        Detail.Severity = Severity;
        Detail.Enabled  = Enabled;
    }
} // namespace Neon::Logger
