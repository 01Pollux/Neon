#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/SSAOPass.hpp>
#include <RenderGraph/Passes/BlurPass.hpp>
#include <RenderGraph/Passes/GBufferPass.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

#include <Core/Random.hpp>

namespace Neon
{
    namespace AssetGuids
    {
        static inline auto SSAOShaderGuid()
        {
            return Asset::Handle::FromString("a9e5e62f-cb0a-41b8-b6a2-318a558d30dd");
        }
    } // namespace AssetGuids
} // namespace Neon

namespace Neon::RG
{
    enum class SSAOPassRS : uint8_t
    {
        FrameData,
        SSAOParams,
        OutputTexture_TextureMap,
    };

    SSAOPass::SSAOPass() :
        RenderPass("SSAOPass")
    {
        GenerateNoise();
        RegenerateSamples();
    }

    void SSAOPass::AddPass(
        GraphBuilder& Builder)
    {
        Builder.AddPass<SSAOPass>();
        auto& Blur = Builder.AddPass<BlurPass>(
            BlurPass::BlurPassData{
                .ViewName = "SSAO",
                .Source   = ResourceId("SSAOOutput"),
                .Output   = ResourceId("AmbientOcclusion") });
    }

    //

    void SSAOPass::RegenerateSamples()
    {
        GenerateSamples();
        if (m_SSAORootSignature) [[unlikely]]
        {
            return;
        }

        // TODO: Load from asset rather than hardcoding
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr Shader(Asset::Manager::LoadAsync(AssetGuids::SSAOShaderGuid()));

        RHI::ShaderCompileDesc ShaderDesc{
            .Stage = RHI::ShaderStage::Compute
        };
        ShaderDesc.Macros.Append(STR("SSAO_SAMPLE_COUNT"), std::to_wstring(SampleCount));

#if NEON_DEBUG
        ShaderDesc.Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

        m_SSAORootSignature =
            RHI::RootSignatureBuilder()
                .AddConstantBufferView("g_FrameData", 0, 0)
                .AddConstantBufferView("c_SSAOParams", 0, 1)
                .AddDescriptorTable(
                    RHI::RootDescriptorTable()
                        .AddUavRangeAt("c_OutputTexture", 0, 1, 1, 0)
                        .AddSrvRangeAt("c_TextureMap", 0, 1, 3, 1, 1))
                .AddStandardSamplers(0, RHI::ShaderVisibility::All)
                .ComputeOnly()
                .Build();

        m_SSAOPipeline =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_SSAORootSignature,
                .ComputeShader = Shader->LoadShader(ShaderDesc)
            }
                .Build();
    }

    float SSAOPass::GetRadius() const noexcept
    {
        return m_Params.Radius;
    }

    void SSAOPass::SetRadius(
        float Val) noexcept
    {
        m_Params.Radius = Val;
    }

    float SSAOPass::GetBias() const noexcept
    {
        return m_Params.Bias;
    }

    void SSAOPass::SetBias(
        float Val) noexcept
    {
        m_Params.Bias = Val;
    }

    float SSAOPass::GetMagnitude() const noexcept
    {
        return m_Params.Magnitude;
    }

    void SSAOPass::SetMagniute(
        float Val) noexcept
    {
        m_Params.Magnitude = Val;
    }

    float SSAOPass::GetContrast() const noexcept
    {
        return m_Params.Contrast;
    }

    void SSAOPass::SetContrast(
        float Val) noexcept
    {
        m_Params.Contrast = Val;
    }

    //

    void SSAOPass::GenerateSamples()
    {
        std::uniform_real_distribution<float> Distribution(0.25f, 1.0f);

        m_Params.Samples[0] = Vector4{ 1.0f, 1.0f, 1.0f, 0.0f };
        m_Params.Samples[1] = Vector4(-1.0f, -1.0f, -1.0f, 0.0f);

        m_Params.Samples[2] = Vector4(-1.0f, 1.0f, 1.0f, 0.0f);
        m_Params.Samples[3] = Vector4(1.0f, -1.0f, -1.0f, 0.0f);

        m_Params.Samples[4] = Vector4(1.0f, 1.0f, -1.0f, 0.0f);
        m_Params.Samples[5] = Vector4(-1.0f, -1.0f, 1.0f, 0.0f);

        m_Params.Samples[6] = Vector4(-1.0f, 1.0f, -1.0f, 0.0f);
        m_Params.Samples[7] = Vector4(1.0f, -1.0f, 1.0f, 0.0f);

        m_Params.Samples[8] = Vector4(-1.0f, 0.0f, 0.0f, 0.0f);
        m_Params.Samples[9] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

        m_Params.Samples[10] = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
        m_Params.Samples[11] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);

        m_Params.Samples[12] = Vector4(0.0f, 0.0f, -1.0f, 0.0f);
        m_Params.Samples[13] = Vector4(0.0f, 0.0f, 1.0f, 0.0f);

        for (size_t i = 0; i < SampleCount; i++)
        {
            auto& Sample = m_Params.Samples[i];
            Sample       = glm::normalize(Sample) * Distribution(Random::GetEngine());
        }
    }

    //

    void SSAOPass::GenerateNoise()
    {
        std::uniform_real_distribution<float> Distribution(0.25f, 1.0f);

        std::vector<Vector4> NoiseData(256 * 256);
        for (auto& Noise : NoiseData)
        {
            Noise.x = Distribution(Random::GetEngine());
            Noise.y = Distribution(Random::GetEngine());
            Noise.z = Distribution(Random::GetEngine());
            Noise.w = 1.f;
        }

        std::array Subresource{
            RHI::ComputeSubresource(
                RHI::EResourceFormat::R32G32B32A32_Float,
                NoiseData.data(),
                256,
                256)
        };

        m_NoiseTexture = RHI::SSyncGpuResource(
            RHI::ResourceDesc::Tex2D(RHI::EResourceFormat::R32G32B32A32_Float, 256, 256, 1),
            Subresource,
            STR("SSAOPass::NoiseTexture"),
            BitMask_Or(RHI::EResourceState::NonPixelShaderResource));
    }

    //

    struct PassResources
    {
        static constexpr uint32_t DescriptorsCount = 3;

        [[nodiscard]] static const ResourceId SSAOOutput()
        {
            return ResourceId{ "SSAOOutput" };
        }

        [[nodiscard]] static const ResourceId GBufferNormal()
        {
            return GBufferPass::GetResource(GBufferPass::ResourceType::Normal);
        }

        [[nodiscard]] static const ResourceId GBufferDepth()
        {
            return GBufferPass::GetResource(GBufferPass::ResourceType::DepthStencil);
        }
    };

    void SSAOPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        const ResourceId SSAOOutputId    = PassResources::SSAOOutput();
        const ResourceId GBufferNormalId = PassResources::GBufferNormal();
        const ResourceId GBufferDepthId  = PassResources::GBufferDepth();

        //

        Resolver.CreateWindowTexture(
            SSAOOutputId,
            RHI::ResourceDesc::Tex2D(RHI::EResourceFormat::R8_UNorm, 1, 1, 1));

        Resolver.WriteResource(
            SSAOOutputId.CreateView("Main"));

        Resolver.ReadTexture(
            GBufferNormalId.CreateView("SSAO"),
            RG::ResourceReadAccess::NonPixelShader);

        Resolver.ReadTexture(
            GBufferDepthId.CreateView("SSAO"),
            RG::ResourceReadAccess::NonPixelShader,
            RHI::SRVDesc{
                .Format = RHI::EResourceFormat::R32_Float,
                .View   = RHI::SRVDesc::Texture2D{} });
    }

    //

    void SSAOPass::PreDispatch(
        const GraphStorage&)
    {
        // Ensure the noise texture is generated
        m_NoiseTexture.WaitForUpload();
    }

    void SSAOPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        const ResourceId SSAOOutputId    = PassResources::SSAOOutput();
        const ResourceId GBufferNormalId = PassResources::GBufferNormal();
        const ResourceId GBufferDepthId  = PassResources::GBufferDepth();

        //

        // We will allocate 3 descriptors  + 1 (noise map)
        auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(PassResources::DescriptorsCount + 1);

        CommandList.SetPipelineState(m_SSAOPipeline);
        CommandList.SetRootSignature(m_SSAORootSignature);

        // Copy to Params' descriptors
        {
            std::array SrcInfo{
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
            };
            static_assert(std::size(SrcInfo) == PassResources::DescriptorsCount);

            RHI::CpuDescriptorHandle
                &SSAOOutput = SrcInfo[0].Descriptor,
                &NormalMap  = SrcInfo[1].Descriptor,
                &DepthMap   = SrcInfo[2].Descriptor;

            Storage.GetResourceView(SSAOOutputId.CreateView("Main"), &SSAOOutput);
            Storage.GetResourceView(GBufferNormalId.CreateView("SSAO"), &NormalMap);
            Storage.GetResourceView(GBufferDepthId.CreateView("SSAO"), &DepthMap);

            Descriptor->Copy(Descriptor.Offset, SrcInfo);

            Descriptor->CreateShaderResourceView(
                Descriptor.Offset + PassResources::DescriptorsCount,
                m_NoiseTexture.Get().get());
        }

        //

        CommandList.SetResourceView(RHI::CstResourceViewType::Cbv, uint32_t(SSAOPassRS::FrameData), Storage.GetFrameDataHandle());
        CommandList.SetDynamicResourceView(RHI::CstResourceViewType::Cbv, uint32_t(SSAOPassRS::SSAOParams), &m_Params, sizeof(m_Params));
        CommandList.SetDescriptorTable(uint32_t(SSAOPassRS::OutputTexture_TextureMap), Descriptor.GetGpuHandle());

        auto Size = Storage.GetOutputImageSize();
        CommandList.Dispatch(Size.x, Size.y);
    }
} // namespace Neon::RG