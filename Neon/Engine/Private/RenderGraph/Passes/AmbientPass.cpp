#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/AmbientPass.hpp>
#include <RenderGraph/Passes/GBufferPass.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

namespace Neon
{
    namespace AssetGuids
    {
        static inline auto AmbientShaderGuid()
        {
            return Asset::Handle::FromString("3c5e4767-680d-43d3-bb45-08312368da19");
        }
    } // namespace AssetGuids
} // namespace Neon

namespace Neon::RG
{
    AmbientPass::AmbientPass() :
        RenderPass("AmbientPass")
    {
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;
        ShaderAssetTaskPtr Shader(Asset::Manager::LoadAsync(AssetGuids::AmbientShaderGuid()));

        RHI::ShaderCompileDesc ShaderDesc{
            .Stage = RHI::ShaderStage::Compute
        };

        RHI::MRootDescriptorTableFlags RsFlags;
        RsFlags.Set(RHI::ERootDescriptorTableFlags::Data_Static_While_Execute);
        RsFlags.Set(RHI::ERootDescriptorTableFlags::Descriptor_Static_Bounds_Check);

        m_AmbientPassRootSignature =
            RHI::RootSignatureBuilder()
                .AddConstantBufferView("g_FrameData", 0, 0)
                .Add32BitConstants<ParamsType>("c_AmbientParams", 0, 1)
                .AddDescriptorTable(
                    RHI::RootDescriptorTable()
                        .AddSrvRange("c_TexturesMap", 0, 1, 4, false, RsFlags)
                        .AddUavRange("c_OutputTexture", 0, 1, 1, false, RsFlags))
                .AddStandardSamplers(0, RHI::ShaderVisibility::All)
                .ComputeOnly()
                .Build();

        m_AmbientPassPipeline =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_AmbientPassRootSignature,
                .ComputeShader = Shader->LoadShader(ShaderDesc)
            }
                .Build();
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