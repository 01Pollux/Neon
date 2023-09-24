#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard.hpp>

#include <RenderGraph/Passes/GBufferPass.hpp>
#include <RenderGraph/Passes/SSAOPass.hpp>
#include <RenderGraph/Passes/AmbientPass.hpp>
#include <RenderGraph/Passes/DebugPass.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

//

#include <Renderer/Render/SpriteRenderer.hpp>
#include <Renderer/Render/MeshRenderer.hpp>

//

namespace Neon::RG
{
    void CreateStandard3DRenderGraph(
        Scene::Component::Camera& CameraComponent,
        const flecs::entity&      Camera)
    {
        /*auto Graph   = CameraComponent.NewRenderGraph(Camera);
        auto Builder = Graph->Reset();

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

        Builder.Build();*/
    }
} // namespace Neon::RG