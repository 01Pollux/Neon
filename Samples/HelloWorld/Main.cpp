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

#include <Logic/PipelineBuilder.hpp>
#include <Logic/Pipeline.hpp>

NEON_MAIN(Argc, Argv)
{
    Neon::PipelineBuilder Builder;

    auto A = Builder.NewPhase("A");
    auto B = Builder.NewPhase("B");
    auto C = Builder.NewPhase("C");
    auto D = Builder.NewPhase("D");

    C.DependsOn(A);
    C.DependsOn(D);
    B.DependsOn(C);

    Neon::Pipeline Pipeline(std::move(Builder));

    Pipeline.Attach("A", []
                    { printf("A\n"); });
    Pipeline.Attach("B", []
                    { printf("B\n"); });
    Pipeline.Attach("C", []
                    { printf("C\n"); });
    Pipeline.Attach("D", []
                    { printf("D\n"); });

    Pipeline.BeginPhases();
    Pipeline.EndPhases();

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
