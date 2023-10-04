#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/GeometryPass.hpp>
#include <RenderGraph/Passes/LightCullPass.hpp>
#include <RenderGraph/Passes/DepthPrepass.hpp>

namespace Neon::RG
{
    GeometryPass::GeometryPass() :
        RenderPass("GeometryPass")
    {
    }

    void GeometryPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        auto Desc = RHI::ResourceDesc::Tex2D(
            Resolver.GetSwapchainFormat(),
            0, 0, 1, 1);

        Desc.ClearValue = RHI::ClearOperation{
            .Format = Resolver.GetSwapchainFormat(),
            .Value  = Colors::Magenta
        };

        Resolver.CreateWindowTexture(
            GeometryPass::ShadedImage,
            Desc);

        //

        Resolver.ReadTexture(
            LightCullPass::LightIndexList_Opaque.CreateView("Geometry"),
            ResourceReadAccess::PixelShader);

        Resolver.ReadTexture(
            LightCullPass::LightIndexList_Transparent.CreateView("Geometry"),
            ResourceReadAccess::PixelShader);

        Resolver.ReadTexture(
            LightCullPass::LightGrid_Opaque.CreateView("Geometry"),
            ResourceReadAccess::PixelShader);

        Resolver.ReadTexture(
            LightCullPass::LightGrid_Transparent.CreateView("Geometry"),
            ResourceReadAccess::PixelShader);

        //

        Resolver.WriteRenderTarget(
            GeometryPass::ShadedImage.CreateView("GeometryPass"),
            RHI::RTVDesc{
                .View      = RHI::RTVDesc::Texture2D{},
                .ClearType = RHI::ERTClearType::Color,
                .Format    = Resolver.GetSwapchainFormat() });

        Resolver.WriteDepthStencil(
            DepthPrepass::DepthBuffer.CreateView("GeometryPass"),
            RHI::DSVDesc{
                .View      = RHI::DSVDesc::Texture2D{},
                .ClearType = RHI::EDSClearType::Ignore,
                .Format    = RHI::EResourceFormat::D32_Float });
    }

    void GeometryPass::DispatchTyped(
        const GraphStorage&      Storage,
        RHI::GraphicsCommandList CommandList)
    {
        auto& SceneContext = Storage.GetSceneContext();
        SceneContext.Render(CommandList, SceneContext::RenderType::RenderPass);
    }
} // namespace Neon::RG