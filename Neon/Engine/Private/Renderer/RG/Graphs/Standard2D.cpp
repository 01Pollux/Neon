#include <EnginePCH.hpp>
#include <Renderer/RG/Graphs/Standard2D.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

//

namespace Neon::RG
{
    UPtr<RG::RenderGraph> CreateStandard2DRenderGraph(
        Scene::GameScene& Scene,
        Scene::Actor      Camera)
    {
        auto Graph   = std::make_unique<RenderGraph>();
        auto Builder = Graph->Reset();

        Builder.AddPass<RG::ScenePass>(Graph->GetStorage(), Scene, Camera);

        Builder.Build();

        return Graph;
    }
} // namespace Neon::RG