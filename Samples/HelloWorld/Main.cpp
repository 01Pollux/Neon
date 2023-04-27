#include <Runtime/GameEngine.hpp>
#include <Log/Logger.hpp>

#include <future>

NEON_MAIN(Argc, Argv)
{
    Neon::Logger::SetLogTag("", Logger::LogSeverity::Warning);
    Neon::Logger::SetLogTag("Window", Logger::LogSeverity::Trace);

    auto func = [](String Name) -> int
    {
        DefaultGameEngine Engine(Config::EngineConfig{
            .Window{
                .Title      = std::move(Name),
                .Windowed   = true,
                .Fullscreen = false } });
        return Engine.Run();
    };

    std::vector<std::future<int>> Engines;

    for (size_t i = 0; i < 1; i++)
    {
        Engines.emplace_back(std::async(func, STR("Engine 1")));
        Engines.emplace_back(std::async(func, STR("Engine 2")));
    }
    return 0;
    /*auto eng1 = std::async(func, STR("Engine 1"));
    auto eng2 = std::async(func, STR("Engine 2"));

    return eng1.get() + eng2.get();*/
}
