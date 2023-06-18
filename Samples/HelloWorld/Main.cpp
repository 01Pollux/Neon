#include <Runtime/GameEngine.hpp>
#include <Resource/Runtime/Manager.hpp>

#include <Log/Logger.hpp>

using namespace Neon;

class TestGameEngine : public Runtime::DefaultGameEngine
{
public:
    TestGameEngine() :
        DefaultGameEngine()
    {
    }

    [[nodiscard]] static Config::EngineConfig GetConfig()
    {
        Config::EngineConfig Config;

        Config.Window.Title      = STR("Test Engine");
        Config.Window.Windowed   = true;
        Config.Window.Fullscreen = false;

        return Config;
    }
};

NEON_MAIN(Argc, Argv)
{
    auto func = []
    {
        TestGameEngine Engine;
        Engine.Initialize(TestGameEngine::GetConfig());
        return Engine.Run();
    };

    return func();

    // auto eng2 = std::async(func);
    // auto eng1 = std::async(func);

    // return eng1.get() /*+ eng2.get()*/;
}
