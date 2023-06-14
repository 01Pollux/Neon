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
    auto D = Builder.NewPhase("D");
    auto E = Builder.NewPhase("E");
    auto C = Builder.NewPhase("C");
    auto K = Builder.NewPhase("K");
    auto F = Builder.NewPhase("F");
    auto L = Builder.NewPhase("L");
    auto J = Builder.NewPhase("J");
    auto M = Builder.NewPhase("M");
    auto N = Builder.NewPhase("N");
    auto Z = Builder.NewPhase("Z");
    auto O = Builder.NewPhase("O");

    C.DependsOn(A);
    C.DependsOn(B);
    F.DependsOn(D);
    F.DependsOn(E);
    J.DependsOn(K);
    J.DependsOn(C);
    J.DependsOn(F);
    M.DependsOn(L);
    M.DependsOn(J);
    N.DependsOn(M);
    Z.DependsOn(N);
    Z.DependsOn(O);

    Neon::Pipeline Pipeline(std::move(Builder));
    Pipeline.SetThreadCount(1);

    // for each phase, call Pipeline.Attach() with callback to printf name
    Pipeline.Attach("A", []
                    { printf("A\n"); });
    Pipeline.Attach("B", []
                    { printf("B\n"); });
    Pipeline.Attach("C", []
                    { printf("C\n"); });
    Pipeline.Attach("D", []
                    { printf("D\n"); });
    Pipeline.Attach("E", []
                    { printf("E\n"); });
    Pipeline.Attach("F", []
                    { printf("F\n"); });
    Pipeline.Attach("J", []
                    { printf("J\n"); });
    Pipeline.Attach("K", []
                    { printf("K\n"); });
    Pipeline.Attach("L", []
                    { printf("L\n"); });
    Pipeline.Attach("M", []
                    { printf("M\n"); });
    Pipeline.Attach("N", []
                    { printf("N\n"); });
    Pipeline.Attach("O", []
                    { printf("O\n"); });
    Pipeline.Attach("Z", []
                    { printf("Z\n"); });

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
