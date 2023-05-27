#pragma once

#include <World/World.hpp>
#include <Config/Engine.hpp>

namespace Neon
{
    namespace Asset
    {
        class IResourceManager;
    } // namespace Asset

    namespace Module
    {
        class Window;
        class ResourceManager;
    } // namespace Module

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

        virtual void Initialize()
        {
        }
        virtual void Shutdown()
        {
        }
        virtual void Tick()
        {
        }

        /// <summary>
        /// Run the engine and return when the engine is closed or an error occurs.
        /// </summary>
        int Run();

    private:
        /// <summary>
        /// Load resource packs
        /// </summary>
        void LoadResourcePacks(
            const Config::ResourceConfig& Config);

    private:
        UPtr<Module::ResourceManager> m_ResourceManager;
        UPtr<Module::Window>          m_Window;
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
