#pragma once

#include <Config/Engine.hpp>

namespace Neon::Module
{
    class ResourceManager;
    class Window;
    class Graphics;
} // namespace Neon::Module

namespace Neon::Runtime
{
    extern int Main(
        int      Argc,
        wchar_t* Argv[]);

    class EnginePipeline;

    class DefaultGameEngine
    {
    public:
        DefaultGameEngine(
            const Config::EngineConfig& Config = {});

        DefaultGameEngine(const DefaultGameEngine&)            = delete;
        DefaultGameEngine& operator=(const DefaultGameEngine&) = delete;

        DefaultGameEngine(DefaultGameEngine&&)             = delete;
        DefaultGameEngine&& operator=(DefaultGameEngine&&) = delete;

        virtual ~DefaultGameEngine();

        /// <summary>
        /// Called when the engine is initialized.
        /// </summary>
        virtual void Initialize()
        {
        }

        /// <summary>
        /// Called when the engine is shutdown.
        /// </summary>
        virtual void Shutdown()
        {
        }

        /// <summary>
        /// Run the engine and return when the engine is closed or an error occurs.
        /// </summary>
        int Run();

    public:
        /// <summary>
        /// Get window associated with the engine
        /// </summary>
        Module::Window* GetWindowModule() noexcept;

        /// <summary>
        /// Get window associated with the engine
        /// </summary>
        Module::Graphics* GetGraphicsModule() noexcept;

        /// <summary>
        /// Get the game pipeline
        /// </summary>
        EnginePipeline& GetPipeline();

        /// <summary>
        /// Get the game pipeline
        /// </summary>
        void SetPipeline(
            UPtr<EnginePipeline> Pipeline);

    private:
        /// <summary>
        /// Load resource packs
        /// </summary>
        void LoadResourcePacks(
            const Config::ResourceConfig& Config);

        /// <summary>
        /// Begin the engine pipeline
        /// </summary>
        void DispatchLoaderPipeline();

    private:
        UPtr<Module::ResourceManager> m_ResourceManager;
        UPtr<Module::Window>          m_Window;

        UPtr<EnginePipeline> m_Pipeline;
    };
} // namespace Neon::Runtime

#if NEON_DIST

#define NEON_MAIN(Argc, Argv)                  \
    int __stdcall wWinMain(                    \
        void*,                                 \
        void*,                                 \
        const wchar_t*,                        \
        int)                                   \
    {                                          \
        Neon::Logger::Initialize();            \
        int Ret = Neon::Main(__argc, __wargv); \
        Neon::Logger::Shutdown();              \
        return Ret;                            \
    }                                          \
    int Neon::Runtime::Main(                   \
        int      Argc,                         \
        wchar_t* Argv[])

#else

#define NEON_MAIN(Argc, Argv)                      \
    int main(                                      \
        int       argc,                            \
        wchar_t** argv)                            \
    {                                              \
        Neon::Logger::Initialize();                \
        int Ret = Neon::Runtime::Main(argc, argv); \
        Neon::Logger::Shutdown();                  \
        return Ret;                                \
    }                                              \
    int Neon::Runtime::Main(                       \
        int      Argc,                             \
        wchar_t* Argv[])

#endif
