#include <EnginePCH.hpp>
#include <Renderer/RG/Graphs/Standard2D.hpp>

//

#include <Renderer/RG/Passes/ScenePass.hpp>

//

namespace Neon::RG
{
    Standard2DRenderGraph::Standard2DRenderGraph(
        Scene::GameScene& Scene)
    {
        auto Builder = Reset();

        Builder.AppendPass<RG::ScenePass>(GetStorage(), Scene);

        Builder.Build();
    }
} // namespace Neon::RG