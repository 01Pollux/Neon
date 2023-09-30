#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/ShadingPass.hpp>

namespace Neon::RG
{
    ShadingPass::ShadingPass() :
        RenderPass("ShadingPass")
    {
    }

    void ShadingPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        auto Desc = RHI::ResourceDesc::Tex2D(
            Resolver.GetSwapchainFormat(),
            0, 0, 1, 1);

        Desc.ClearValue = RHI::ClearOperation{
            .Format = Resolver.GetSwapchainFormat(),
            .Value  = Colors::Magenta
        };

        const ResourceId ShadedImage("ShadedImage");
        const ResourceId DepthBuffer("DepthBuffer");

        Resolver.CreateWindowTexture(
            ShadedImage,
            Desc);

        Resolver.ReadResourceEmpty(
            DepthBuffer);

        Resolver.WriteRenderTarget(
            ShadedImage.CreateView("ShadingPass"),
            RHI::RTVDesc{
                .View      = RHI::RTVDesc::Texture2D{},
                .ClearType = RHI::ERTClearType::Color,
                .Format    = Resolver.GetSwapchainFormat() });

        Resolver.WriteDepthStencil(
            DepthBuffer.CreateView("ShadingPass"),
            RHI::DSVDesc{
                .View      = RHI::DSVDesc::Texture2D{},
                .ClearType = RHI::EDSClearType::Ignore,
                .Format    = RHI::EResourceFormat::D32_Float });
    }

    void ShadingPass::DispatchTyped(
        const GraphStorage&      Storage,
        RHI::GraphicsCommandList CommandList)
    {
        auto  CameraStorage = Storage.GetFrameDataHandle();
        auto& SceneContext  = Storage.GetSceneContext();

        SceneContext.Render(CommandList, SceneContext::RenderType::RenderPass);
    }
} // namespace Neon::RG