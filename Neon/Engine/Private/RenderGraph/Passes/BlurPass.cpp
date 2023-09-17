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
    enum class BlurPassRS : uint8_t
    {
        OutputIndexOffset,
        BlurParams,
        InputOutput_TextureMap
    };

    BlurPass::BlurPass(
        BlurPassData Data) :
        RenderPass("BlurPass"),
        m_Data(std::move(Data))
    {
        SetSigma(2.5f);
        m_Iterations = 3;

        // TODO: Load from asset rather than hardcoding
        using ShaderAssetTaskPtr = Asset::AssetTaskPtr<Asset::ShaderAsset>;

        ShaderAssetTaskPtr Shader(Asset::Manager::LoadAsync(AssetGuids::BlurShaderGuid()));

        RHI::ShaderCompileDesc ShaderDesc{
            .Stage = RHI::ShaderStage::Compute
        };

#if NEON_DEBUG
        ShaderDesc.Flags.Set(RHI::EShaderCompileFlags::Debug);
#endif

        m_BlurSubPassRootSignature =
            RHI::RootSignatureBuilder()
                .Add32BitConstants<uint32_t>("c_OutputIndexOffset", 0, 1)
                .Add32BitConstants<GaussWeightsList>("c_BlurParams", 1, 1)
                .AddDescriptorTable(
                    RHI::RootDescriptorTable()
                        .AddUavRange("c_InputOutput", 0, 1, 3))
                .ComputeOnly()
                .Build();

        m_BlurSubPassPipelineStateV =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_BlurSubPassRootSignature,
                .ComputeShader = Shader->LoadShader(ShaderDesc)
            }
                .Build();

        ShaderDesc.Macros.Append(STR("BLUR_H"));

        m_BlurSubPassPipelineStateH =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_BlurSubPassRootSignature,
                .ComputeShader = Shader->LoadShader(ShaderDesc)
            }
                .Build();
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

        m_GaussWeights = { 0.0002f,
                           0.0060f,
                           0.0606f,
                           0.2417f,
                           0.3829f,
                           0.2417f,
                           0.0606f,
                           0.0060f,
                           0.0002f };
    }

    float BlurPass::GetSigma() const noexcept
    {
        return m_Sigma;
    }

    //

    void BlurPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        const ResourceId Intermediate(STR("BlurSubPassIntermediate_") + m_Data.ViewName);

        const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
        const ResourceViewId IntermediateOutputView(Intermediate.CreateView(m_Data.ViewName));
        const ResourceViewId OutputView(m_Data.Output.CreateView(m_Data.ViewName));

        auto [Desc, Flags] = Resolver.GetResourceDescAndFlags(m_Data.Source);

        Resolver.CreateTexture(
            Intermediate,
            *Desc,
            Flags);

        Resolver.CreateTexture(
            m_Data.Output,
            *Desc,
            Flags);

        Resolver.ReadResourceEmpty(m_Data.Source);
        Resolver.WriteResource(SourceView, m_Data.OutputDesc);
        Resolver.WriteResource(IntermediateOutputView, m_Data.OutputDesc);
        Resolver.WriteResource(OutputView, m_Data.OutputDesc);
    }

    void BlurPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        const ResourceId Intermediate(STR("BlurSubPassIntermediate_") + m_Data.ViewName);

        const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
        const ResourceViewId IntermediateOutputView(Intermediate.CreateView(m_Data.ViewName));
        const ResourceViewId OutputView(m_Data.Output.CreateView(m_Data.ViewName));

        //

        // We will allocate 4 descriptors, one for input and one for output for each stage
        auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(3);

        auto IntermediateResource = Storage.GetResource(Intermediate).Get().get();

        // Stage: Input(Index), Output(Index)
        // H: Input(0), Output(1)
        // V: Input(1), Output(2)
        {
            std::array SrcInfo{
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 }
            };

            RHI::CpuDescriptorHandle
                &InputH  = SrcInfo[0].Descriptor,
                &OutputH = SrcInfo[1].Descriptor,
                &OutputV = SrcInfo[2].Descriptor;

            Storage.GetResourceView(SourceView, &InputH);
            Storage.GetResourceView(IntermediateOutputView, &OutputH);
            Storage.GetResourceView(OutputView, &OutputV);

            Descriptor->Copy(Descriptor.Offset, SrcInfo);
        }

        CommandList.SetRootSignature(m_BlurSubPassRootSignature);
        CommandList.SetConstants(uint32_t(BlurPassRS::BlurParams), m_GaussWeights);
        CommandList.SetDescriptorTable(uint32_t(BlurPassRS::InputOutput_TextureMap), Descriptor.GetGpuHandle());

        auto Size = Storage.GetResourceSize(m_Data.Source);

        for (uint32_t i = 0; i < m_Iterations; i++)
        {
            CommandList.SetPipelineState(m_BlurSubPassPipelineStateH);
            CommandList.SetConstants(uint32_t(BlurPassRS::OutputIndexOffset), 0);
            CommandList.Dispatch(Math::DivideByMultiple(Size.x, KernelSize), Size.y);

            CommandList.InsertUAVBarrier(IntermediateResource);

            CommandList.SetPipelineState(m_BlurSubPassPipelineStateV);
            CommandList.SetConstants(uint32_t(BlurPassRS::OutputIndexOffset), 1);
            CommandList.Dispatch(Size.x, Math::DivideByMultiple(Size.y, KernelSize));
        }
    }
} // namespace Neon::RG
