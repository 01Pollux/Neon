#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard.hpp>

#include <RenderGraph/Passes/DepthPrepass.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

//

namespace Neon::RG
{
    void CreateStandard3DRenderGraph(
        Scene::Component::Camera& CameraComponent,
        const flecs::entity&      Camera)
    {
        auto Graph   = CameraComponent.NewRenderGraph(Camera);
        auto Builder = Graph->Reset();

        Builder.AddPass<DepthPrepass>();
        Builder.Build();
        /*
        auto& GBuffer = Builder.AddPass<GBufferPass>();
        {
            GBuffer.AttachRenderer<Renderer::SpriteRenderer>();
            GBuffer.AttachRenderer<Renderer::MeshRenderer>();
        }

        ResourceId FinalImage = GBufferPass::GetResource(GBufferPass::ResourceType::Normal);

        if (false)
        {
            SSAOPass::AddPass(Builder);
            Builder.AddPass<AmbientPass>();

#ifndef NEON_DIST
            Builder.AddPass<DebugPass>(ResourceId("HdrRenderTarget"));
#endif

            FinalImage = ResourceId("HdrRenderTarget");
        }

        Builder.AddPass<CopyToTexturePass>(
            CopyToTexturePass::CopyToTextureData{
                .ViewName    = "Present",
                .Source      = FinalImage,
                .Destination = ResourceResolver::GetOutputImage() });

        */
    }
} // namespace Neon::RG