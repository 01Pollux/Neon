#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/DepthPrepass.hpp>

namespace Neon::RG
{
    DepthPrepass::DepthPrepass() :
        RenderPass("DepthPrepass")
    {
    }

    void DepthPrepass::ResolveResources(
        ResourceResolver& Resolver)
    {
        auto Desc = RHI::ResourceDesc::Tex2D(
            RHI::EResourceFormat::R32_Typeless,
            0, 0, 1, 1);

        Desc.ClearValue = RHI::ClearOperation{
            .Format = RHI::EResourceFormat::D32_Float,
            .Value  = RHI::ClearOperation::DepthStencil{ 1.0f, 0 }
        };

        ResourceId DepthBuffer("DepthBuffer");

        Resolver.CreateWindowTexture(
            DepthBuffer,
            Desc);

        Resolver.WriteDepthStencil(
            DepthBuffer.CreateView("DepthPrePass"),
            RHI::DSVDesc{
                .View      = RHI::DSVDesc::Texture2D{},
                .ClearType = RHI::EDSClearType::Depth,
                .Format    = RHI::EResourceFormat::D32_Float });
    }

    void DepthPrepass::DispatchTyped(
        const GraphStorage&      Storage,
        RHI::GraphicsCommandList CommandList)
    {
        auto  CameraStorage = Storage.GetFrameDataHandle();
        auto& SceneContext  = Storage.GetSceneContext();

        SceneContext.Render(CommandList, SceneContext::RenderType::DepthPrepass);
    }
} // namespace Neon::RG