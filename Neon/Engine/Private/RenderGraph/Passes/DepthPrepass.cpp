#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/DepthPrepass.hpp>

namespace Neon::RG
{
    DepthPrepass::DepthPrepass() :
        RenderPass("DepthPrepass")
    {
    }

    UPtr<IRenderPass> DepthPrepass::Clone()
    {
        return std::make_unique<DepthPrepass>(*this);
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

        Resolver.CreateWindowTexture(
            DepthPrepass::DepthBuffer,
            Desc);

        Resolver.WriteDepthStencil(
            DepthPrepass::DepthBuffer.CreateView("DepthPrePass"),
            RHI::DSVDesc{
                .View      = RHI::DSVDesc::Texture2D{},
                .ClearType = RHI::EDSClearType::Depth,
                .Format    = RHI::EResourceFormat::D32_Float });
    }

    void DepthPrepass::DispatchTyped(
        const GraphStorage&      Storage,
        RHI::GraphicsCommandList CommandList)
    {
        auto& SceneContext = Storage.GetSceneContext();
        SceneContext.Render(CommandList, SceneContext::RenderType::DepthPrepass, {}, {});
    }
} // namespace Neon::RG