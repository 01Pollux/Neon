#pragma once

#include <Runtime/GameEngine.hpp>
#include <Config/Engine.hpp>
#include <Asset/Manager.hpp>
#include <Asset/Storage.hpp>

#include <ranges>

#include <Log/Logger.hpp>

namespace Neon::Runtime
{
    /// <summary>
    /// Entry point for the engine.
    /// </summary>
    extern UPtr<GameEngine> Main(
        int       Argc,
        wchar_t** Argv);
}; // namespace Neon::Runtime

#if defined(NEON_PLATFORM_WINDOWS) && defined(NEON_DIST)

#define NEON_MAIN(Argc, Argv)                                  \
    int __stdcall wWinMain(                                    \
        void*,                                                 \
        void*,                                                 \
        const wchar_t*,                                        \
        int)                                                   \
    {                                                          \
        Neon::Logger::Initialize();                            \
        std::atexit(&Neon::Logger::Shutdown);                  \
        Neon::Asset::Storage::Initialize();                    \
                                                               \
        auto Engine = Neon::Runtime::Main(__argc, __wargv);    \
        int  Ret    = Engine->Run();                           \
        Engine.reset();                                        \
                                                               \
        Neon::Asset::Storage::Shutdown();                      \
        return Ret;                                            \
    }                                                          \
    Neon::UPtr<Neon::Runtime::GameEngine> Neon::Runtime::Main( \
        int       Argc,                                        \
        wchar_t** Argv)

#else

#define NEON_MAIN(Argc, Argv)                                  \
    int wmain(                                                 \
        int       argc,                                        \
        wchar_t** argv)                                        \
    {                                                          \
        Neon::Logger::Initialize();                            \
        std::atexit(&Neon::Logger::Shutdown);                  \
        Neon::Asset::Storage::Initialize();                    \
                                                               \
        auto Engine = Neon::Runtime::Main(argc, argv);         \
        int  Ret    = Engine->Run();                           \
        Engine.reset();                                        \
                                                               \
        Neon::Asset::Storage::Shutdown();                      \
        return Ret;                                            \
    }                                                          \
    Neon::UPtr<Neon::Runtime::GameEngine> Neon::Runtime::Main( \
        int       Argc,                                        \
        wchar_t** Argv)

#endif

namespace Neon::Runtime
{
    template<typename _Ty>
        requires std::is_base_of_v<GameEngine, _Ty>
    UPtr<GameEngine> RunEngine(
        Config::EngineConfig Config)
    {
        auto Engine = std::make_unique<_Ty>();
        Engine->Initialize(std::move(Config));
        return Engine;
    }
} // namespace Neon::Runtime
