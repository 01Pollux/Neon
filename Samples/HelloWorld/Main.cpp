#include <Runtime/EntryPoint.hpp>

using namespace Neon;

class TestGameEngine : public Runtime::DefaultGameEngine
{
public:
    TestGameEngine() :
        DefaultGameEngine()
    {
    }
};

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
