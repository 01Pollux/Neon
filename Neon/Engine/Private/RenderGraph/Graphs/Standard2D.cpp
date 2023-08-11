#include <EnginePCH.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

#include <RenderGraph/Passes/GBufferPass.hpp>
#include <RenderGraph/Passes/DebugPass.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

//

#include <Renderer/Render/SpriteRenderer.hpp>

//

namespace Neon::RG
{
    UPtr<RenderGraph> CreateStandard2DRenderGraph(
        flecs::entity Camera)
    {
        auto Graph   = std::make_unique<RenderGraph>();
        auto Builder = Graph->Reset();

        auto& GBuffer = Builder.AddPass<GBufferPass>(Camera);
        {
            GBuffer.AttachRenderer<Renderer::SpriteRenderer>();
        }

#ifndef NEON_DIST
        // Builder.AddPass<RG::DebugPass>();
#endif

        Builder.Build();

        return Graph;
    }
} // namespace Neon::RG