#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/BlurPass.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

namespace Neon
{
    namespace AssetGuids
    {
        static inline auto BlurShaderGuid()
        {
            return Asset::Handle::FromString("8d605fba-0199-4716-90ca-600366176e8a");
        }
    } // namespace AssetGuids
} // namespace Neon

namespace Neon::RG
{
    BlurPass::BlurPass(
        BlurPassData Data) :
        RenderPass("BlurPass"),
        m_Data(std::move(Data))
    {
        SetSigma(2.5f);

        // TODO: Load from asset rather than hardcoding
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr Shader(Asset::Manager::LoadAsync(AssetGuids::BlurShaderGuid()));

        RHI::ShaderCompileDesc ShaderDesc{
            .Stage = RHI::ShaderStage::Compute
        };

#if NEON_DEBUG
        ShaderDesc.Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

        m_BlurPassRootSignature =
            RHI::RootSignatureBuilder()
                .Add32BitConstants<GaussWeightsList>("c_BlurParams", 0, 1)
                .AddDescriptorTable(
                    RHI::RootDescriptorTable()
                        .AddUavRangeAt("c_Input", 0, 1, 1, 0)
                        .AddSrvRangeAt("c_Output", 0, 1, 1, 1))
                .ComputeOnly()
                .Build();

        {
            m_BlurPassPipelineStateV =
                RHI::PipelineStateBuilderC{
                    .RootSignature = m_BlurPassRootSignature,
                    .ComputeShader = Shader->LoadShader(ShaderDesc)
                }
                    .Build();
        }
        {
            ShaderDesc.Macros.Append(STR("BLUR_H"));

            m_BlurPassPipelineStateH =
                RHI::PipelineStateBuilderC{
                    .RootSignature = m_BlurPassRootSignature,
                    .ComputeShader = Shader->LoadShader(ShaderDesc)
                }
                    .Build();
        }
    }

    //

    void BlurPass::SetSigma(
        float Sigma)
    {
        m_Sigma = Sigma;

        float TwoSigmaSquared = Sigma * Sigma * 2.0f;

        float TotalWeight = 0.0f;

        constexpr int Radius = BlurGaussWeightCount / 2;
        for (int i = -Radius; i <= Radius; i++)
        {
            float Weight               = std::expf(-float(i * i) / TwoSigmaSquared);
            m_GaussWeights[i + Radius] = Weight;
            TotalWeight += Weight;
        }

        for (size_t i = 0; i < BlurGaussWeightCount; i++)
        {
            m_GaussWeights[i] /= TotalWeight;
        }
    }

    float BlurPass::GetSigma() const noexcept
    {
        return m_Sigma;
    }

    //

    void BlurPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        const ResourceId Intermediate(STR("BlurPassIntermediate_") + m_Data.ViewName);

        const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
        const ResourceViewId IntermediateOutput(Intermediate.CreateView(STR("O_") + m_Data.ViewName));
        const ResourceViewId IntermediateInput(Intermediate.CreateView(STR("I_") + m_Data.ViewName));
        const ResourceViewId OutputView(m_Data.Output.CreateView(m_Data.ViewName));

        //

        auto [Desc, Flags] = Resolver.GetResourceDescAndFlags(m_Data.Source);

        Resolver.CreateTexture(
            Intermediate,
            *Desc,
            Flags);
        Resolver.CreateTexture(
            m_Data.Output,
            *Desc,
            Flags);

        Resolver.ReadTexture(SourceView, ResourceReadAccess::NonPixelShader, m_Data.SourceDesc);
        Resolver.WriteResource(IntermediateOutput);

        Resolver.ReadTexture(IntermediateInput, ResourceReadAccess::NonPixelShader, m_Data.SourceDesc);
        Resolver.WriteResource(OutputView, m_Data.OutputDesc);
    }

    void BlurPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        const ResourceId Intermediate(STR("BlurPassIntermediate_") + m_Data.ViewName);

        const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
        const ResourceViewId IntermediateOutput(Intermediate.CreateView(STR("O_") + m_Data.ViewName));
        const ResourceViewId IntermediateInput(Intermediate.CreateView(STR("I_") + m_Data.ViewName));
        const ResourceViewId OutputView(m_Data.Output.CreateView(m_Data.ViewName));

        //
        constexpr size_t DescriptorCount = 4;

        // We will allocate 4 descriptors, one for input and one for output for each stage
        auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(DescriptorCount);

        // Sage: Type(Index), Type(Stage)
        // H: Input(0), Output(1)
        // V: Input(2), Output(3)
        {
            std::array SrcInfo{
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 }
            };
            static_assert(std::size(SrcInfo) == DescriptorCount);

            RHI::CpuDescriptorHandle
                &InputH  = SrcInfo[0].Descriptor,
                &OutputH = SrcInfo[1].Descriptor,
                &InputV  = SrcInfo[2].Descriptor,
                &OutputV = SrcInfo[3].Descriptor;

            Storage.GetResourceView(SourceView, &InputH);
            Storage.GetResourceView(IntermediateOutput, &OutputH);
            Storage.GetResourceView(IntermediateInput, &InputV);
            Storage.GetResourceView(OutputView, &OutputV);

            Descriptor->Copy(Descriptor.Offset, SrcInfo);
        }

        CommandList.SetRootSignature(m_BlurPassRootSignature);
        CommandList.SetConstants(0, m_GaussWeights);

        auto Size = Storage.GetResourceSize(m_Data.Source);

        for (uint32_t i = 0; i < m_Iterations; i++)
        {
            CommandList.SetPipelineState(m_BlurPassPipelineStateH);
            CommandList.SetDescriptorTable(1, Descriptor.GetGpuHandle());
            CommandList.Dispatch(Math::DivideByMultiple(Size.x, KernelSize), Size.y);

            CommandList.SetPipelineState(m_BlurPassPipelineStateV);
            CommandList.SetDescriptorTable(1, Descriptor.GetGpuHandle(2));
            CommandList.Dispatch(Size.x, Math::DivideByMultiple(Size.y, KernelSize));
        }
    }
} // namespace Neon::RG