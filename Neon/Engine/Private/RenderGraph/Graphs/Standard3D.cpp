#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard.hpp>

#include <RenderGraph/Passes/DepthPrepass.hpp>
#include <RenderGraph/Passes/GridFrustumPass.hpp>
#include <RenderGraph/Passes/GeometryPass.hpp>
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

        // Builder.AddPass<LightCullPass>();

        Builder.AddPass<GeometryPass>();

        Builder.AddPass<CopyToTexturePass>(
            CopyToTexturePass::CopyToTextureData{
                .ViewName    = "Finalize",
                .Source      = ResourceId("ShadedImage"),
                .Destination = ResourceResolver::GetOutputImage() });

        Builder.Build();
    }
} // namespace Neon::RG