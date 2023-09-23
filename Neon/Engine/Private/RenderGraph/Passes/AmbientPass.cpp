#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/AmbientPass.hpp>
#include <RenderGraph/Passes/GBufferPass.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/GlobalDescriptors.hpp>

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
    enum class AmbientPassRS : uint8_t
    {
        FrameData,
        AmbientParams,
        OutputTexture_TextureMap,
    };

    AmbientPass::AmbientPass() :
        RenderPass("AmbientPass")
    {
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;
        ShaderAssetTaskPtr Shader(Asset::Manager::LoadAsync(AssetGuids::AmbientShaderGuid()));

        RHI::ShaderCompileDesc ShaderDesc{
            .Stage = RHI::ShaderStage::Compute
        };

#if NEON_DEBUG
        ShaderDesc.Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

        m_AmbientRootSignature =
            RHI::RootSignatureBuilder()
                .AddConstantBufferView("g_FrameData", 0, 0)
                .Add32BitConstants<ParamsType>("c_AmbientParams", 0, 1)
                .AddDescriptorTable(
                    RHI::RootDescriptorTable()
                        .AddUavRangeAt("c_OutputTexture", 0, 1, 1, 0)
                        .AddSrvRangeAt("c_TextureMap", 0, 1, 4, 1))
                .AddStandardSamplers(0, RHI::ShaderVisibility::All)
                .ComputeOnly()
                .Build();

        m_AmbientPipeline =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_AmbientRootSignature,
                .ComputeShader = Shader->LoadShader(ShaderDesc)
            }
                .Build();
    }

    struct PassResources
    {
        static constexpr size_t DescriptorsCount = 5;

        [[nodiscard]] static const ResourceId HdrRenderTarget()
        {
            return ResourceId{ "HdrRenderTarget" };
        }

        [[nodiscard]] static const ResourceId AmbientOcclusion()
        {
            return ResourceId{ "AmbientOcclusion" };
        }

        [[nodiscard]] static const ResourceId GBufferAlbedo()
        {
            return GBufferPass::GetResource(GBufferPass::ResourceType::Albedo);
        }

        [[nodiscard]] static const ResourceId GBufferEmissive()
        {
            return GBufferPass::GetResource(GBufferPass::ResourceType::Emissive);
        }

        [[nodiscard]] static const ResourceId GBufferDepth()
        {
            return GBufferPass::GetResource(GBufferPass::ResourceType::DepthStencil);
        }
    };

    void AmbientPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        const ResourceId HdrRenderTargetId  = PassResources::HdrRenderTarget();
        const ResourceId GBufferAlbedoId    = PassResources::GBufferAlbedo();
        const ResourceId GBufferEmissiveId  = PassResources::GBufferEmissive();
        const ResourceId GBufferDepthId     = PassResources::GBufferDepth();
        const ResourceId AmbientOcclusionId = PassResources::AmbientOcclusion();

        //

        Resolver.CreateWindowTexture(
            HdrRenderTargetId,
            RHI::ResourceDesc::Tex2D(RHI::EResourceFormat::R16G16B16A16_Float, 1, 1, 1));

        Resolver.WriteResource(
            HdrRenderTargetId.CreateView("Main"));

        Resolver.ReadTexture(
            GBufferAlbedoId.CreateView("Ambient"),
            ResourceReadAccess::NonPixelShader);

        Resolver.ReadTexture(
            GBufferEmissiveId.CreateView("Ambient"),
            ResourceReadAccess::NonPixelShader);

        Resolver.ReadTexture(
            GBufferDepthId.CreateView("Ambient"), ResourceReadAccess::NonPixelShader,
            RHI::SRVDesc{
                .Format = RHI::EResourceFormat::R32_Float,
                .View   = RHI::SRVDesc::Texture2D{} });

        Resolver.ReadTexture(
            AmbientOcclusionId.CreateView("Ambient"), ResourceReadAccess::NonPixelShader);
    }

    void AmbientPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        const ResourceId HdrRenderTargetId  = PassResources::HdrRenderTarget();
        const ResourceId GBufferAlbedoId    = PassResources::GBufferAlbedo();
        const ResourceId GBufferEmissiveId  = PassResources::GBufferEmissive();
        const ResourceId GBufferDepthId     = PassResources::GBufferDepth();
        const ResourceId AmbientOcclusionId = PassResources::AmbientOcclusion();

        //

        auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(PassResources::DescriptorsCount);

        CommandList.SetPipelineState(m_AmbientPipeline);
        CommandList.SetRootSignature(m_AmbientRootSignature);

        // Copy to Params' descriptors
        {
            std::array SrcInfo{
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 }
            };
            static_assert(std::size(SrcInfo) == PassResources::DescriptorsCount);

            RHI::CpuDescriptorHandle
                &OutputTexture       = SrcInfo[0].Descriptor,
                &AlbedoMap           = SrcInfo[1].Descriptor,
                &EmissiveFactorMap   = SrcInfo[2].Descriptor,
                &DepthMap            = SrcInfo[3].Descriptor,
                &AmbientOcclusionMap = SrcInfo[4].Descriptor;

            Storage.GetResourceView(HdrRenderTargetId.CreateView("Main"), &OutputTexture);
            Storage.GetResourceView(GBufferAlbedoId.CreateView("Ambient"), &AlbedoMap);
            Storage.GetResourceView(GBufferEmissiveId.CreateView("Ambient"), &EmissiveFactorMap);
            Storage.GetResourceView(GBufferDepthId.CreateView("Ambient"), &DepthMap);
            Storage.GetResourceView(AmbientOcclusionId.CreateView("Ambient"), &AmbientOcclusionMap);

            Descriptor->Copy(Descriptor.Offset, SrcInfo);
        }

        //

        ParamsType Params{
            .Color = ColorToU32(Colors::White)
        };

        CommandList.SetResourceView(RHI::CstResourceViewType::Cbv, uint32_t(AmbientPassRS::FrameData), Storage.GetFrameDataHandle());
        CommandList.SetConstants<ParamsType>(uint32_t(AmbientPassRS::AmbientParams), Params);
        CommandList.SetDescriptorTable(uint32_t(AmbientPassRS::OutputTexture_TextureMap), Descriptor.GetGpuHandle());

        auto Size = Storage.GetOutputImageSize();
        CommandList.Dispatch(Size.x, Size.y);
    }
} // namespace Neon::RG