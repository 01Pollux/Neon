#include <EnginePCH.hpp>
#include <Runtime/Phases/SplashScreen.hpp>

#include <Runtime/Pipeline.hpp>
#include <Runtime/GameEngine.hpp>

#include <Module/Graphics.hpp>
#include <Renderer/RG/Passes/Lambda.hpp>

namespace Neon::Runtime::Phases
{
    static constexpr const char* SplashScreenTag = "SplashScreen";

    void SplashScreen::Build(
        EnginePipelineBuilder& Builder)
    {
        //
    }

    void SplashScreen::Bind(
        DefaultGameEngine* Engine)
    {
        auto& Pipeline = Engine->GetPipeline();

        auto Graphics    = Engine->GetGraphicsModule();
        auto RenderGraph = Graphics->GetRenderGraph();

        auto Builder = RenderGraph->Reset();

        Builder.AppendPass<RG::LambdaPass>(RG::PassQueueType::Direct)
            .SetDispatcher(
                [](const RG::GraphStorage& Storage, RHI::ICommandList* CommandList) {
                });

        Builder.Build();
    }
} // namespace Neon::Runtime::Phases