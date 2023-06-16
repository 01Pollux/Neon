#include <EnginePCH.hpp>
#include <Runtime/Phases/SplashScreen.hpp>
#include <Runtime/Pipeline.hpp>
#include <Runtime/GameEngine.hpp>

namespace Neon::Runtime::Phases
{
    static constexpr const char* SplashScreenTag = "SplashScreen";

    void SplashScreen::Build(
        EnginePipelineBuilder& Builder)
    {
        auto RenderPhase       = Builder.GetPhase("Render");
        auto SplashScreenPhase = Builder.NewPhase(SplashScreenTag);

        SplashScreenPhase.DependsOn(RenderPhase);

        //
    }

    void SplashScreen::Bind(
        DefaultGameEngine* Engine)
    {

        auto& Pipeline = Engine->GetPipeline();
        Pipeline.Attach(
            SplashScreenTag,
            [] {

            });
    }
} // namespace Neon::Runtime::Phases