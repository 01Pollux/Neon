#pragma once

#include <World/World.hpp>
#include <Config/Engine.hpp>

namespace Neon
{
    namespace RHI
    {
        class IRenderDevice;
    }

    namespace Asset
    {
        class IResourceManager;
    }

    extern int Main(
        int      Argc,
        wchar_t* Argv[]);

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
        /// Run the engine and return when the engine is closed or an error occurs.
        /// </summary>
        int Run();

        /// <summary>
        /// Get engine's current world.
        /// </summary>
        World& GetWorld();

    private:
        /// <summary>
        /// Create window application
        /// </summary>
        void CreateWindow(
            const Config::WindowConfig& Config);

        /// <summary>
        /// Load resource packs
        /// </summary>
        void LoadResourcePacks(
            const Config::ResourceConfig& Config);

    private:
        World m_World;

        UPtr<RHI::IRenderDevice> m_RenderDevice;
    };
} // namespace Neon

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
    int Neon::Main(                            \
        int      Argc,                         \
        wchar_t* Argv[])

#else

#define NEON_MAIN(Argc, Argv)             \
    int main(                             \
        int       argc,                   \
        wchar_t** argv)                   \
    {                                     \
        Neon::Logger::Initialize();       \
        int Ret = Neon::Main(argc, argv); \
        Neon::Logger::Shutdown();         \
        return Ret;                       \
    }                                     \
    int Neon::Main(                       \
        int      Argc,                    \
        wchar_t* Argv[])

#endif
