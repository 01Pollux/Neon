#pragma once

#include <Config/Engine.hpp>
#include <Runtime/GameEngine.hpp>
#include <Log/Logger.hpp>
#include <ranges>

namespace Neon::Runtime
{
    /// <summary>
    /// Entry point for the engine.
    /// </summary>
    extern int Main(
        int       Argc,
        wchar_t** Argv);
}; // namespace Neon::Runtime

#if defined(NEON_PLATFORM_WINDOWS) && defined(NEON_DIST)

#define NEON_MAIN(Argc, Argv)                      \
    int __stdcall wWinMain(                        \
        void*,                                     \
        void*,                                     \
        const wchar_t*,                            \
        int)                                       \
    {                                              \
        Neon::Logger::Initialize();                \
        std::atexit(&Neon::Logger::Shutdown);      \
        int Ret = Neon::Runtime::Main(argc, argv); \
        return Ret;                                \
    }                                              \
    int Neon::Runtime::Main(                       \
        int       Argc,                            \
        wchar_t** Argv)

#else

#define NEON_MAIN(Argc, Argv)                      \
    int wmain(                                     \
        int       argc,                            \
        wchar_t** argv)                            \
    {                                              \
        Neon::Logger::Initialize();                \
        std::atexit(&Neon::Logger::Shutdown);      \
        int Ret = Neon::Runtime::Main(argc, argv); \
        return Ret;                                \
    }                                              \
    int Neon::Runtime::Main(                       \
        int       Argc,                            \
        wchar_t** Argv)

#endif

namespace Neon::Runtime
{
    template<typename _Ty>
        requires std::is_base_of_v<DefaultGameEngine, _Ty>
    int RunEngine(
        const Config::EngineConfig& Config)
    {
        _Ty Engine{};
        Engine.Initialize(Config);
        return Engine.Run();
    }
} // namespace Neon::Runtime
