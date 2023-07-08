#include <Runtime/EntryPoint.hpp>

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false }
    };
    return RunEngine<Runtime::DefaultGameEngine>(Config);
}