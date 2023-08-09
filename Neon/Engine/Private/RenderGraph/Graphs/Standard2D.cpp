#include <EnginePCH.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>
#include <RenderGraph/Passes/ScenePass.hpp>

//

#include <Renderer/Render/SpriteRenderer.hpp>

//

namespace Neon::RG
{
    UPtr<RG::RenderGraph> CreateStandard2DRenderGraph(
        flecs::entity Camera)
    {
        auto Graph   = std::make_unique<RenderGraph>();
        auto Builder = Graph->Reset();

        Builder.AddPass<RG::ScenePass>(Camera)
            .AttachRenderer<Renderer::SpriteRenderer>();

        Builder.Build();

        return Graph;
    }
} // namespace Neon::RG