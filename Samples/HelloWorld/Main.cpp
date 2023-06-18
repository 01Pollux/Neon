#include <Runtime/EntryPoint.hpp>

using namespace Neon;

class TestGameEngine : public Runtime::DefaultGameEngine
{
public:
};

#include <flecs/flecs.h>

NEON_MAIN(Argc, Argv)
{
    Config::EngineConfig Config{
        .Window = {
            .Title      = STR("Test Engine"),
            .Windowed   = true,
            .Fullscreen = false }
    };
    return RunEngine<TestGameEngine>(Config);
}
