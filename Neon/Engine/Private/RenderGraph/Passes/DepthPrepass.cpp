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
            RHI::EResourceFormat::R24G8_Typeless,
            0, 0, 1, 1);

        Desc.ClearValue = RHI::ClearOperation{
            .Format = RHI::EResourceFormat::D24_UNorm_S8_UInt,
            .Value  = Colors::Black
        };

        Resolver.CreateWindowTexture(
            ResourceId("DepthBuffer"),
            Desc);
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