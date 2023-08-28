#include <EnginePCH.hpp>

#include <Script/Internal/Engine.hpp>
#include <Private/Script/Internal/Assembly.hpp>
#include <Private/Script/HandleManager.hpp>

#include <Mono/jit/jit.h>
#include <Mono/metadata/appdomain.h>
#include <Mono/metadata/attrdefs.h>
#include <Mono/metadata/assembly.h>
#include <Mono/metadata/exception.h>
#include <Mono/metadata/mono-debug.h>
#include <Mono/metadata/mono-config.h>
#include <Mono/metadata/threads.h>
#include <Mono/metadata/tokentype.h>
#include <Mono/metadata/debug-helpers.h>

#include <Mono/utils/mono-logger.h>

#include <Log/Logger.hpp>
#include <fstream>

namespace Neon::Scripting
{
    struct ScriptContext
    {
        MonoDomain* RootDomain{};
        MonoDomain* CurrentDomain{};

        HandleManager HandleMan;

        std::map<StringU8, CS::Assembly> LoadedAssemblies;

        /// <summary>
        /// Create a new domain.
        /// </summary>
        void NewDomain();

        bool IsMonoInitialized : 1 = false;
    } static s_ScriptContext;

    //

    HandleManager* HandleManager::Get()
    {
        return &s_ScriptContext.HandleMan;
    }

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
        mono_trace_set_print_handler(
            [](const char* Text, mono_bool)
            {
                NEON_WARNING_TAG("Script", Text);
            });
        mono_trace_set_printerr_handler(
            [](const char* Text, mono_bool)
            {
                NEON_ERROR_TAG("Script", Text);
            });

        s_ScriptContext.RootDomain = mono_jit_init_version("NeonRoot", Config.Version);
        NEON_VALIDATE(s_ScriptContext.RootDomain, "Scripting engine failed to initialize.");

#ifdef NEON_DEBUG
        mono_debug_domain_create(s_ScriptContext.RootDomain);
#endif

        mono_thread_set_main(mono_thread_current());

        s_ScriptContext.IsMonoInitialized = true;
        NEON_TRACE_TAG("Script", "Scripting engine initialized.");

        s_ScriptContext.NewDomain();
    }

    void Shutdown()
    {
        if (s_ScriptContext.IsMonoInitialized)
        {
            NEON_TRACE_TAG("Script", "Scripting engine shutting down.");

            s_ScriptContext.HandleMan.Shutdown();
            mono_jit_cleanup(s_ScriptContext.RootDomain);
            s_ScriptContext = {};
        }
        else
        {
            NEON_WARNING_TAG("Script", "Scripting engine was not initialized.");
        }
    }

    //

    static MonoAssembly* LoadAssemblyFromFile(
        const char* Path)
    {
        std::ifstream     File(Path, std::ios::binary);
        std::vector<char> FileData((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());

        MonoImageOpenStatus Status;

        MonoImage* Image = mono_image_open_from_data_full(FileData.data(), uint32_t(FileData.size()), true, &Status, false);
        NEON_VALIDATE(Status == MONO_IMAGE_OK, "Failed to open C# image: {}.", mono_image_strerror(Status));

        MonoAssembly* Assembly = mono_assembly_load_from_full(Image, Path, &Status, 0);
        NEON_VALIDATE(Status == MONO_IMAGE_OK, "Failed to open C# assembly: {}.", mono_image_strerror(Status));

        mono_image_close(Image);
        return Assembly;
    }

    void LoadAssembly(
        const char*     Name,
        const StringU8& Path)
    {
        std::ifstream File(Path, std::ios::binary);
        NEON_VALIDATE(File.is_open(), "Failed to open C# assembly: {}.", Path);

        s_ScriptContext.LoadedAssemblies.emplace(Name, CS::Assembly(Name, File));
    }

    void UnloadAssembly(
        const char* Name)
    {
        s_ScriptContext.LoadedAssemblies.erase(Name);
    }

    //

    void ScriptContext::NewDomain()
    {
        char        DomainName[] = "Neon";
        MonoDomain* Domain       = mono_domain_create_appdomain(DomainName, nullptr);
        NEON_VALIDATE(Domain, "Failed to create domain.");

        mono_domain_set(Domain, true);
        if (CurrentDomain)
        {
            mono_domain_unload(CurrentDomain);
        }

        CurrentDomain = Domain;
    }
} // namespace Neon::Scripting