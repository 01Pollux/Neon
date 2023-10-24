#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Graphs/Standard.hpp>

#include <RenderGraph/Passes/DepthPrepass.hpp>
#include <RenderGraph/Passes/LightCullPass.hpp>
#include <RenderGraph/Passes/GeometryPass.hpp>
#include <RenderGraph/Passes/CopyToTexture.hpp>

//

namespace Neon::RG
{
    void CreateStandard3DRenderGraph(
        RG::RenderGraph* Rendergraph)
    {
        auto Builder = Rendergraph->Reset();

        Builder.AddPass<DepthPrepass>();

        Builder.AddPass<LightCullPass>();

        Builder.AddPass<GeometryPass>();

        Builder.AddPass<CopyToTexturePass>(
            CopyToTexturePass::CopyToTextureData{
                .ViewName    = "Finalize",
                .Source      = GeometryPass::ShadedImage,
                .Destination = ResourceResolver::GetOutputImage() });

        Builder.Build();
    }
} // namespace Neon::RG