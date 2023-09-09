#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/SSAOPass.hpp>

#include <Renderer/Material/Material.hpp>
#include <Renderer/Material/Builder.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

#include <Core/Random.hpp>

namespace Neon
{
    namespace AssetGuids
    {
        static inline auto CopyToTextureShaderGuid()
        {
            return Asset::Handle::FromString("a9e5e62f-cb0a-41b8-b6a2-318a558d30dd");
        }
    } // namespace AssetGuids
} // namespace Neon

namespace Neon::RG
{
    SSAOPass::SSAOPass(
        uint32_t SampleCount) :
        RenderPass(STR("SSAOPass"))
    {
        RegenerateSamples(SampleCount);
    }

    //

    void SSAOPass::RegenerateSamples(
        uint32_t SampleCount)
    {
        if (SampleCount && m_Params.Samples.size() != SampleCount)
        {
            m_Params.Samples.resize(SampleCount);
            GenerateSamples();
        }

        if (!SampleCount)
        {
            GenerateSamples();
            return;
        }

        // TODO: Load from asset rather than hardcoding
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr Shader = Asset::Manager::Load(AssetGuids::CopyToTextureShaderGuid());

        RHI::ShaderCompileDesc ShaderDesc{
            .Stage = RHI::ShaderStage::Compute
        };
        ShaderDesc.Macros.Append(STR("SAMPLE_COUNT"), std::to_wstring(SampleCount));

        Renderer::ComputeMaterialBuilder Builder;
        m_Material =
            Builder.RootSignature(
                       RHI::RootSignatureBuilder()
                           .AddConstantBufferView("c_PerFrameData", 0, 0)
                           .AddConstantBufferView("c_SSAOParams", 1, 0)
                           .AddDescriptorTable(
                               RHI::RootDescriptorTable()
                                   .AddSrvRange("c_NormalMap", 0, 0, 1)
                                   .AddSrvRange("c_DepthMap", 1, 0, 1)
                                   .AddSrvRange("c_NoiseMap", 2, 0, 1)
                                   .AddUavRange("c_OcclusionOutput", 0, 0, 1))
                           .AddStandardSamplers()
                           .Build())
                .ComputeShader(Shader->LoadShader(ShaderDesc))
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

    float SSAOPass::GetResolutionFactor() const noexcept
    {
        return m_Params.ResolutionFactor;
    }

    void SSAOPass::SetResolutionFactor(
        float Val) noexcept
    {
        m_Params.ResolutionFactor = Val;
    }

    //

    void SSAOPass::GenerateSamples()
    {
        std::uniform_real_distribution<float> Distribution(0.0f, 1.0f);

        for (auto& Sample : m_Params.Samples)
        {
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
            RHI::SubresourceDesc{
                .Data       = NoiseData.data(),
                .RowPitch   = 8 * sizeof(Vector4),
                .SlicePitch = 0 }
        };

        auto Desc = RHI::ResourceDesc::Tex2D(
            RHI::EResourceFormat::R32G32B32A32_Float,
            8,
            8,
            1);

        m_NoiseTexture = RHI::USyncTexture(Desc, Subresource, BitMask_Or(RHI::EResourceState::NonPixelShaderResource));
    }

    //

    void SSAOPass::ResolveResources(
        ResourceResolver& Resolver)
    {
    }

    void SSAOPass::DispatchTyped(
        const GraphStorage&       Storage,
        RHI::IComputeCommandList* CommandList)
    {
    }
} // namespace Neon::RG