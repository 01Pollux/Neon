#include <EnginePCH.hpp>
#include <Renderer/RG/Graphs/Standard2D.hpp>

//

#include <Scene/Scene.hpp>
#include <Renderer/RG/Passes/ScenePass.hpp>

//

namespace Neon::RG
{
    UPtr<RG::RenderGraph> CreateStandard2DRenderGraph(
        Scene::GameScene& Scene)
    {
        auto Graph   = std::make_unique<RenderGraph>();
        auto Builder = Graph->Reset();

        Builder.AppendPass<RG::ScenePass>(Graph->GetStorage(), Scene);

        Builder.Build();

        return Graph;
    }
} // namespace Neon::RG