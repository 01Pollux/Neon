#include <EnginePCH.hpp>

#include <Script/Engine.hpp>

#include <Mono/jit/jit.h>
#include <Mono/metadata/appdomain.h>
#include <Mono/metadata/assembly.h>
#include <Mono/metadata/exception.h>
#include <Mono/metadata/mono-debug.h>
#include <Mono/metadata/mono-config.h>
#include <Mono/metadata/threads.h>

#include <Mono/utils/mono-logger.h>

#include <Log/Logger.hpp>

namespace Neon::Scripting
{
    void Test();

    struct ScriptContext
    {
        MonoDomain* Domain{};

        bool IsMonoInitialized : 1 = false;
    } static s_ScriptContext;

    //

    static void OnScriptLog(
        const char* LogDomain,
        const char* LogLevel,
        const char* Message,
        mono_bool,
        void*)
    {
        static std::map<StringU8, Logger::LogSeverity> Severities{
            { "error", Logger::LogSeverity::Error },
            { "critical", Logger::LogSeverity::Fatal },
            { "warning", Logger::LogSeverity::Warning },
            { "message", Logger::LogSeverity::Info },
            { "info", Logger::LogSeverity::Info },
            { "debug", Logger::LogSeverity::Trace }
        };

        Logger::LogSeverity Severity = Logger::LogSeverity::Info;

        auto Iter = Severities.find(LogLevel);
        if (Iter != Severities.end()) [[likely]]
        {
            Severity = Iter->second;
        }

        if (Logger::ShouldLog(Severity))
        {
            Logger::LogMessage(Severity, Message);
        }
    }

    //

    void Initialize(
        const Config::ScriptConfig& Config)
    {
        mono_set_dirs(".", ".");

        constexpr const char* MonoConfig = R"(
			<configuration>
        		<dllmap dll="i:cygwin1.dll" target="@LIBC@" os="!windows" />
        		<dllmap dll="libc" target="libc.so.6" os="!windows" />
			</configuration>
		)";

        mono_config_parse_memory(MonoConfig);

#ifndef NEON_DIST
        {
            auto Debugger = StringUtils::Format(
                "--debugger-agent=transport=dt_socket,"
                "suspend=n,"
                "server=y,"
                "address=127.0.0.1:{},"
                "embedding=1",
                Config.Port);

            const char* Options[]{
                "--soft-breakpoints",
                Debugger.c_str(),
                "--debug-domain-unload",
                "--gc-debug=check-remset-consistency,verify-before-collections,xdomain-checks"
            };

            mono_jit_parse_options(int(std::size(Options)), const_cast<char**>(Options));
            mono_debug_init(MONO_DEBUG_FORMAT_MONO);
        }
#ifdef NEON_DEBUG
        mono_trace_set_level_string("debug");
#else
        mono_trace_set_level_string("info");
#endif
#else
        mono_trace_set_level_string("warning");
#endif

        mono_trace_set_log_handler(OnScriptLog, nullptr);
        s_ScriptContext.Domain = mono_jit_init_version("Neon", Config.Version);
        NEON_VALIDATE(s_ScriptContext.Domain, "Scripting engine failed to initialize.");

        mono_thread_set_main(mono_thread_current());

        s_ScriptContext.IsMonoInitialized = true;
        NEON_TRACE_TAG("Script", "Scripting engine initialized.");

        Test();
    }

    void Shutdown()
    {
        if (s_ScriptContext.IsMonoInitialized)
        {
            NEON_TRACE_TAG("Script", "Scripting engine shutting down.");
            mono_jit_cleanup(s_ScriptContext.Domain);
            s_ScriptContext = {};
        }
        else
        {
            NEON_WARNING_TAG("Script", "Scripting engine was not initialized.");
        }
    }

    //

    void Test()
    {

    }
} // namespace Neon::Scripting