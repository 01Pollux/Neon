#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/SSAOPass.hpp>
#include <RenderGraph/Passes/GBufferPass.hpp>

#include <Renderer/Material/Material.hpp>
#include <Renderer/Material/Builder.hpp>

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
    SSAOPass::SSAOPass(
        uint32_t SampleCount) :
        RenderPass("SSAOPass")
    {
        GenerateNoise();
        RegenerateSamples(SampleCount);
    }

    void RG::SSAOPass::AddPass(
        GraphBuilder& Builder)
    {
        Builder.AddPass<SSAOPass>();
    }

    //

    void SSAOPass::RegenerateSamples(
        uint32_t SampleCount)
    {
        if (SampleCount && m_SampleCount != SampleCount)
        {
            m_SampleCount = SampleCount;
            GenerateSamples();
        }

        if (!SampleCount)
        {
            GenerateSamples();
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

        Renderer::ComputeMaterialBuilder Builder;
        m_Material =
            Builder.RootSignature(
                       RHI::RootSignatureBuilder(STR("SSAOPass::RootSignature"))
                           .AddConstantBufferView("c_PerFrameData", 0, 0)
                           .AddConstantBufferView("c_SSAOParams", 1, 0)
                           .AddDescriptorTable(
                               RHI::RootDescriptorTable()
                                   .AddSrvRange("c_NormalMap", 0, 0, 1)
                                   .AddSrvRange("c_DepthMap", 1, 0, 1)
                                   .AddSrvRange("c_NoiseMap", 2, 0, 1)
                                   .AddUavRange("c_OcclusionOutput", 0, 0, 1))
                           .AddStandardSamplers(0, RHI::ShaderVisibility::All)
                           .Build())
                .ComputeShader(Shader->LoadShader(std::move(ShaderDesc)))
                .Build();
    }

    float SSAOPass::GetRadius() const noexcept
    {
        return GetSSAOParams()->Radius;
    }

    void SSAOPass::SetRadius(
        float Val) noexcept
    {
        GetSSAOParams()->Radius = Val;
    }

    float SSAOPass::GetBias() const noexcept
    {
        return GetSSAOParams()->Bias;
    }

    void SSAOPass::SetBias(
        float Val) noexcept
    {
        GetSSAOParams()->Bias = Val;
    }

    float SSAOPass::GetMagnitude() const noexcept
    {
        return GetSSAOParams()->Magnitude;
    }

    void SSAOPass::SetMagniute(
        float Val) noexcept
    {
        GetSSAOParams()->Magnitude = Val;
    }

    float SSAOPass::GetContrast() const noexcept
    {
        return GetSSAOParams()->Contrast;
    }

    void SSAOPass::SetContrast(
        float Val) noexcept
    {
        GetSSAOParams()->Contrast = Val;
    }

    float SSAOPass::GetResolutionFactor() const noexcept
    {
        return GetSSAOParams()->ResolutionFactor;
    }

    void SSAOPass::SetResolutionFactor(
        float Val) noexcept
    {
        GetSSAOParams()->ResolutionFactor = Val;
    }

    //

    void SSAOPass::GenerateSamples()
    {
        std::uniform_real_distribution<float> Distribution(0.0f, 1.0f);

        {
            auto NewPtr(std::make_unique<uint8_t[]>(GetParamsSize()));
            if (m_Params)
            {
                new (NewPtr.get()) ParamsType(*GetSSAOParams());
            }
            else
            {
                new (NewPtr.get()) ParamsType;
            }
            m_Params = std::move(NewPtr);
        }
        for (size_t i = 0; i < m_SampleCount; i++)
        {
            auto& Sample = GetSSAOSamples()[i];

            Sample.x = 2.f * Distribution(Random::GetEngine()) - 1.f;
            Sample.y = 2.f * Distribution(Random::GetEngine()) - 1.f;
            Sample.z = Distribution(Random::GetEngine());
            Sample.w = 0.0f;
            Sample   = glm::normalize(Sample) * Distribution(Random::GetEngine());
        }
    }

    //

    void SSAOPass::GenerateNoise()
    {
        std::uniform_real_distribution<float> Distribution(0.0f, 1.0f);

        std::vector<Vector4> NoiseData(8 * 8);
        for (auto& Noise : NoiseData)
        {
            Noise.x = Distribution(Random::GetEngine());
            Noise.y = Distribution(Random::GetEngine());
            Noise.z = Distribution(Random::GetEngine());
            Noise.w = Distribution(Random::GetEngine());
        }

        std::array Subresource{
            RHI::ComputeSubresource(
                RHI::EResourceFormat::R32G32B32A32_Float,
                NoiseData.data(),
                8,
                8)
        };

        m_NoiseTexture = RHI::SSyncTexture(
            RHI::ResourceDesc::Tex2D(RHI::EResourceFormat::R32G32B32A32_Float, 8, 8, 1),
            Subresource,
            BitMask_Or(RHI::EResourceState::NonPixelShaderResource));
    }

    //

    void SSAOPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        auto           SSAODesc = RHI::ResourceDesc::Tex2D(RHI::EResourceFormat::R8_UNorm, 1, 1, 1);
        RG::ResourceId SSAOOutput(STR("SSAOOutput"));

        Resolver.CreateWindowTexture(
            SSAOOutput,
            SSAODesc);

        Resolver.ReadTexture(
            GBufferPass::GetResource(GBufferPass::ResourceType::Normal).CreateView(STR("SSAO")),
            RG::ResourceReadAccess::NonPixelShader);

        Resolver.ReadTexture(
            GBufferPass::GetResource(GBufferPass::ResourceType::DepthStencil).CreateView(STR("SSAO")),
            RG::ResourceReadAccess::NonPixelShader,
            RHI::SRVDesc{
                .Format = RHI::EResourceFormat::R32_Float,
                .View   = RHI::SRVDesc::Texture2D{} });

        Resolver.WriteResource(
            SSAOOutput.CreateView(STR("Main")));
    }

    //

    void SSAOPass::PreDispatch(
        const GraphStorage&)
    {
        // Ensure the noise texture is generated
        m_NoiseTexture.WaitForUpload();
    }

    void SSAOPass::DispatchTyped(
        const GraphStorage& Storage,
        RHI::ICommandList*  CommandList)
    {
        auto GBufferNormalId = GBufferPass::GetResource(GBufferPass::ResourceType::Normal);
        auto GBufferDepthId  = GBufferPass::GetResource(GBufferPass::ResourceType::DepthStencil);
        auto SSAOOutputId    = RG::ResourceId(STR("SSAOOutput"));

        //

        auto& NormalMap    = Storage.GetResource(GBufferNormalId);
        auto& DepthMap     = Storage.GetResource(GBufferDepthId);
        auto& SSAOOutput   = Storage.GetResource(SSAOOutputId);
        auto& DepthMapView = std::get<RHI::SRVDescOpt>(Storage.GetResourceView(GBufferDepthId.CreateView(STR("SSAO"))));

        //

        auto ParamsType = GetSSAOParams();

        m_Material->SetDynamicResourceView(
            "c_SSAOParams",
            RHI::CstResourceViewType::Cbv,
            m_Params.get(),
            GetParamsSize());

        m_Material->SetResourceView(
            "c_PerFrameData",
            Storage.GetFrameDataHandle());

        m_Material->SetTexture(
            "c_NormalMap",
            NormalMap.Get());

        m_Material->SetTexture(
            "c_DepthMap",
            DepthMap.Get(),
            DepthMapView);

        m_Material->SetTexture(
            "c_NoiseMap",
            m_NoiseTexture.Get());

        m_Material->SetUnorderedAcess(
            "c_OcclusionOutput",
            SSAOOutput.Get());

        m_Material->Apply(CommandList);

        //

        float Factor = GetSSAOParams()->ResolutionFactor;

        auto Size = Storage.GetOutputImageSize();
        Size.x    = int(Size.x * Factor);
        Size.y    = int(Size.y * Factor);

        CommandList->Dispatch2D(Size.x, Size.y, m_SampleCount, m_SampleCount);
    }
} // namespace Neon::RG