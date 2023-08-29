#include <EnginePCH.hpp>

#include <Private/Script/Internal/Engine.hpp>
#include <Script/Internal/Utils.hpp>
#include <Script/Internal/Class.hpp>

#include <Mono/utils/mono-logger.h>
#include <Log/Logger.hpp>
#include <fstream>

namespace Neon::Scripting
{
    namespace CS
    {
        static ScriptContext s_ScriptContext;
        ScriptContext*       ScriptContext::Get()
        {
            return &s_ScriptContext;
        }

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
    } // namespace CS

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

        CS::ScriptContext::Get()->RootDomain = mono_jit_init_version("NeonRoot", Config.Version);
        NEON_VALIDATE(CS::ScriptContext::Get()->RootDomain, "Scripting engine failed to initialize.");

#ifdef NEON_DEBUG
        mono_debug_domain_create(CS::ScriptContext::Get()->RootDomain);
#endif

        mono_thread_set_main(mono_thread_current());

        CS::ScriptContext::Get()->IsMonoInitialized = true;
        NEON_TRACE_TAG("Script", "Scripting engine initialized.");

        CS::ScriptContext::Get()->NewDomain();
    }

    void Shutdown()
    {
        if (CS::ScriptContext::Get()->IsMonoInitialized)
        {
            NEON_TRACE_TAG("Script", "Scripting engine shutting down.");

            CS::ScriptContext::Get()->HandleMgr.Shutdown();
            mono_jit_cleanup(CS::ScriptContext::Get()->RootDomain);
            CS::s_ScriptContext = {};
        }
        else
        {
            NEON_WARNING_TAG("Script", "Scripting engine was not initialized.");
        }
    }

    //

    GCHandle CreateScriptObject(
        const char*            AssemblyName,
        const char*            TypeName,
        std::span<const char*> ParameterTypes,
        const void**           Parameters,
        uint32_t               ParameterCount)
    {
        auto Cls = GetClass(AssemblyName, TypeName);
        if (!Cls) [[unlikely]]
        {
            NEON_ERROR_TAG("Script", "Failed to find type: {}.", TypeName);
            return {};
        }

        CS::Object Obj = ParameterCount ? Cls->New(ParameterTypes, Parameters, ParameterCount) : Cls->New();
        if (!Obj) [[unlikely]]
        {
            NEON_ERROR_TAG("Script", "Failed to create object: {}.", TypeName);
            return {};
        }

        return CS::ScriptContext::Get()->HandleMgr.AddReference(Obj.GetObject());
    }

    const CS::Class* GetClass(
        const char* AssemblyName,
        const char* TypeName)
    {
        auto AssemblyIter = CS::ScriptContext::Get()->LoadedAssemblies.find(AssemblyName);
        if (AssemblyIter == CS::ScriptContext::Get()->LoadedAssemblies.end()) [[unlikely]]
        {
            NEON_ERROR_TAG("Script", "Failed to find assembly: {}.", AssemblyName);
            return {};
        }

        auto& Assembly = AssemblyIter->second;
        return Assembly.GetClass(TypeName);
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

        CS::ScriptContext::Get()->LoadedAssemblies.emplace(Name, CS::Assembly(Name, File));
    }

    void UnloadAssembly(
        const char* Name)
    {
        CS::ScriptContext::Get()->LoadedAssemblies.erase(Name);
    }
} // namespace Neon::Scripting