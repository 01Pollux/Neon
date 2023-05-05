#pragma once

#include <World/World.hpp>
#include <Config/Engine.hpp>

namespace Neon
{
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

    protected:
        /// <summary>
        /// Get resource manager
        /// </summary>
        [[nodiscard]] Asset::IResourceManager* GetResourceManager();

    private:
        /// <summary>
        /// Create window application
        /// </summary>
        void CreateWindow(
            const Config::WindowConfig& Config);

        /// <summary>
        /// Load resource packs
        /// </summary>
        void LoadResourcePacks();

    private:
        World m_World;

        Ptr<Asset::IResourceManager> m_AssetManager;
    };
} // namespace Neon

#if NEON_DIST

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define NEON_MAIN(Argc, Argv)             \
    int WINAPI wWinMain(                  \
        HINSTANCE hInstance,              \
        HINSTANCE hPrevInstance,          \
        LPTSTR    lpCmdLine,              \
        int       nShowCmd)               \
    {                                     \
        Neon::Logger::Initialize();       \
        int Ret = Neon::Main(argc, argv); \
        Neon::Logger::Shutdown();         \
        return Ret;                       \
    }                                     \
    int Neon::Main(                       \
        int      Argc,                    \
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
