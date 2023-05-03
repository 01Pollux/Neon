#define _CRT_SECURE_NO_WARNINGS
#include <Runtime/GameEngine.hpp>
#include <Log/Logger.hpp>

#include <future>

#include <Core/SHA256.hpp>

class TestGameEngine : public Neon::DefaultGameEngine
{
public:
    TestGameEngine() :
        Neon::DefaultGameEngine(GetConfig())
    {
    }

private:
    [[nodiscard]] static Neon::Config::EngineConfig GetConfig()
    {
        Neon::Config::EngineConfig Config;
        Config.Window.Title      = L"Test Engine";
        Config.Window.Windowed   = true;
        Config.Window.Fullscreen = false;
        return Config;
    }
};

NEON_MAIN(Argc, Argv)
{
    Neon::Logger::SetLogTag("", Logger::LogSeverity::Warning);
    Neon::Logger::SetLogTag("Window", Logger::LogSeverity::Trace);

    auto func = []
    {
        TestGameEngine Engine;
        return Engine.Run();
    };

    auto eng1 = std::async(func);

    return eng1.get();
}
