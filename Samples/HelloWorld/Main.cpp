#include <Runtime/GameEngine.hpp>
#include <Log/Logger.hpp>

#include <future>
#include <semaphore>

NEON_MAIN(Argc, Argv)
{
    auto func = [](String Name) -> int
    {
        Config::EngineConfig Config{
            .Window{
                .Title = std::move(Name) }
        };

        int ExitCode;
        {
            DefaultGameEngine Engine(Config);
            ExitCode = Engine.Run();
        }
        return ExitCode;
    };

    auto eng1 = std::async(func, STR("Engine 1"));
    auto eng2 = std::async(func, STR("Engine 2"));

    return eng1.get() + eng2.get();
}
