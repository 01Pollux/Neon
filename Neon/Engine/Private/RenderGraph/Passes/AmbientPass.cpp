#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/AmbientPass.hpp>
#include <RenderGraph/Passes/GBufferPass.hpp>

namespace Neon::RG
{
    AmbientPass::AmbientPass() :
        RenderPass("AmbientPass")
    {
    }

    void AmbientPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        const ResourceId HdrRenderTarget(STR("HdrRenderTarget"));

        const ResourceId AmbientOcclusion(STR("SSAOOutput"));
        // const ResourceId AmbientOcclusion(STR("AmbientOcclusion"));
        const ResourceId GBufferAlbedo(GBufferPass::GetResource(GBufferPass::ResourceType::Albedo));
        const ResourceId GBufferNormal(GBufferPass::GetResource(GBufferPass::ResourceType::Normal));
        const ResourceId GBufferEmissive(GBufferPass::GetResource(GBufferPass::ResourceType::Emissive));
        const ResourceId GBufferDepth(GBufferPass::GetResource(GBufferPass::ResourceType::DepthStencil));

        Resolver.CreateTexture(
            HdrRenderTarget,
            RHI::ResourceDesc::Tex2D(RHI::EResourceFormat::R16G16B16A16_Float, 1, 1, 1));

        Resolver.ReadTexture(GBufferAlbedo.CreateView(STR("Ambient")), ResourceReadAccess::NonPixelShader);
        Resolver.ReadTexture(GBufferNormal.CreateView(STR("Ambient")), ResourceReadAccess::NonPixelShader);
        Resolver.ReadTexture(GBufferEmissive.CreateView(STR("Ambient")), ResourceReadAccess::NonPixelShader);
        Resolver.ReadTexture(GBufferDepth.CreateView(STR("Ambient")), ResourceReadAccess::NonPixelShader,
                             RHI::SRVDesc{
                                 .Format = RHI::EResourceFormat::R32_Float,
                                 .View   = RHI::SRVDesc::Texture2D{} });
        Resolver.ReadTexture(AmbientOcclusion.CreateView(STR("Ambient")), ResourceReadAccess::NonPixelShader);
        Resolver.WriteResource(HdrRenderTarget.CreateView(STR("Main")));
    }

    void AmbientPass::DispatchTyped(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
    }
} // namespace Neon::RG