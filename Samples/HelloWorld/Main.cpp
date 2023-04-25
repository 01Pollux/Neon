#include <Runtime/GameEngine.hpp>
#include <Log/Logger.hpp>

#include <future>

NEON_MAIN(Argc, Argv)
{
    auto func = [](String Name) -> int
    {
        DefaultGameEngine Engine(Config::EngineConfig{
            .Window{
                .Title      = std::move(Name),
                .Windowed   = true,
                .Fullscreen = false } });
        return Engine.Run();
    };

    // auto eng1 = std::async(func, STR("Engine 1"));
    auto eng2 = std::async(func, STR("Engine 2"));

    return /*eng1.get() +*/ eng2.get();
}
