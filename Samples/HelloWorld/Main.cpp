#include <Runtime/GameEngine.hpp>
#include <Module/Resource.hpp>
#include <Resource/Runtime/Manager.hpp>

#include <Log/Logger.hpp>

#include <future>

using namespace Neon;

class TestGameEngine : public Runtime::DefaultGameEngine
{
public:
    TestGameEngine() :
        DefaultGameEngine(GetConfig())
    {
    }

private:
    [[nodiscard]] static Config::EngineConfig GetConfig()
    {
        Config::EngineConfig Config;

        Config.Window.Title      = STR("Test Engine");
        Config.Window.Windowed   = true;
        Config.Window.Fullscreen = false;

        Config.Resource.Manager = NEON_NEW Asset::RuntimeResourceManager;
        return Config;
    }
};

NEON_MAIN(Argc, Argv)
{
    auto func = []
    {
        TestGameEngine Engine;
        return Engine.Run();
    };

    return func();

    // auto eng2 = std::async(func);
    // auto eng1 = std::async(func);

    // return eng1.get() /*+ eng2.get()*/;
}
