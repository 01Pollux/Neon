#include <EnginePCH.hpp>

#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/GeometryPass.hpp>
#include <RenderGraph/Passes/LightCullPass.hpp>
#include <RenderGraph/Passes/DepthPrepass.hpp>

namespace Neon::RG
{
    enum class LightDataRS_Resources : uint8_t
    {
        Lights_0,
        LightIndexList_Opaque,
        LightGrid_Opaque,

        Lights_1,
        LightIndexList_Transparent,
        LightGrid_Transparent,

        Count,

        OpaqueStart      = Lights_0,
        TransparentStart = Lights_1
    };

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
        auto  Descriptor   = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(uint32_t(LightDataRS_Resources::Count));

        std::array Sources{
            // Lights_0
            RHI::IDescriptorHeap::CopyInfo{
                .Descriptor = SceneContext.GetLightsResourceView(),
                .CopySize   = 1 },
            // LightIndexList_Opaque
            RHI::IDescriptorHeap::CopyInfo{
                .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightIndexList_Opaque.CreateView("Geometry")),
                .CopySize   = 1 },
            // LightGrid_Opaque
            RHI::IDescriptorHeap::CopyInfo{
                .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightGrid_Transparent.CreateView("Geometry")),
                .CopySize   = 1 },
            // Lights_1
            RHI::IDescriptorHeap::CopyInfo{
                .Descriptor = SceneContext.GetLightsResourceView(),
                .CopySize   = 1 },
            // LightIndexList_Transparent
            RHI::IDescriptorHeap::CopyInfo{
                .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightIndexList_Transparent.CreateView("Geometry")),
                .CopySize   = 1 },
            // LightGrid_Transparent
            RHI::IDescriptorHeap::CopyInfo{
                .Descriptor = Storage.GetResourceViewHandle(LightCullPass::LightGrid_Transparent.CreateView("Geometry")),
                .CopySize   = 1 }
        };

        Descriptor->Copy(
            Descriptor.Offset,
            Sources);

        SceneContext.Render(
            CommandList,
            SceneContext::RenderType::RenderPass,
            Descriptor.GetGpuHandle(uint32_t(LightDataRS_Resources::OpaqueStart)),
            Descriptor.GetGpuHandle(uint32_t(LightDataRS_Resources::TransparentStart)));
    }
} // namespace Neon::RG