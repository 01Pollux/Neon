#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard2D.hpp>

#include <RenderGraph/Passes/GBufferPass.hpp>
#include <RenderGraph/Passes/DebugPass.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

//

#include <Renderer/Render/SpriteRenderer.hpp>

//

namespace Neon::RG
{
    void CreateStandard2DRenderGraph(
        Scene::Component::Camera& CameraComponent,
        const flecs::entity&      Camera)
    {
        auto Graph   = CameraComponent.NewRenderGraph(Camera);
        auto Builder = Graph->Reset();

        auto& GBuffer = Builder.AddPass<GBufferPass>();
        {
            GBuffer.AttachRenderer<Renderer::SpriteRenderer>();
        }

#ifndef NEON_DIST
        Builder.AddPass<DebugPass>(ResourceId(STR("GBufferAlbedo")));
#endif

        Builder.Build();
    }
} // namespace Neon::RG