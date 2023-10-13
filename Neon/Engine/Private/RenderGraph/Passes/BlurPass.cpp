#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/BlurPass.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <RHI/Shaders/Blur.hpp>

namespace Neon::RG
{
    enum class BlurPassRS : uint8_t
    {
        IndexOffset,
        BlurParams,
        InputOutput_TextureMap
    };

    BlurPass::BlurPass(
        BlurPassData Data) :
        RenderPass("BlurPass"),
        m_Data(std::move(Data))
    {
        SetSigma(2.5f);

        RHI::Shaders::BlurShader Shader;

        RHI::ShaderCompileDesc ShaderDesc{
            .Stage = RHI::ShaderStage::Compute
        };

        auto RootSignature = RHI::IRootSignature::Get(RHI::RSCommon::Type::BlurPass);

        m_BlurSubPassPipelineStateV =
            RHI::PipelineStateBuilderC{
                .RootSignature = RootSignature,
                .ComputeShader = Shader->LoadShader(ShaderDesc)
            }
                .Build();

        ShaderDesc.Macros.Append(STR("BLUR_H"));

        m_BlurSubPassPipelineStateH =
            RHI::PipelineStateBuilderC{
                .RootSignature = RootSignature,
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

    void RG::BlurPass::SetIterations(
        uint32_t Iterations)
    {
        m_Iterations = Iterations;
    }

    uint32_t RG::BlurPass::GetIterations() const noexcept
    {
        return m_Iterations;
    }

    //

    void BlurPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        const ResourceId Intermediate("BlurSubPassIntermediate_" + m_Data.ViewName);

        const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
        const ResourceViewId IntermediateOutputView(Intermediate.CreateView(m_Data.ViewName));
        const ResourceViewId OutputView(m_Data.Output.CreateView(m_Data.ViewName));

        //

        auto [Desc, Flags] = Resolver.GetResourceDescAndFlags(m_Data.Source);

        Resolver.CreateResource(
            Intermediate,
            *Desc,
            Flags);

        Resolver.CreateResource(
            m_Data.Output,
            *Desc,
            Flags);

        // We will write to source <=> tmp, then to the output in the final iteration
        Resolver.ReadResourceEmpty(m_Data.Source);
        Resolver.WriteResource(SourceView, m_Data.OutputDesc);
        Resolver.WriteResource(IntermediateOutputView, m_Data.OutputDesc);
        Resolver.WriteResource(OutputView, m_Data.OutputDesc);
    }

    void BlurPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        const ResourceId Intermediate("BlurSubPassIntermediate_" + m_Data.ViewName);

        const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
        const ResourceViewId IntermediateOutputView(Intermediate.CreateView(m_Data.ViewName));
        const ResourceViewId OutputView(m_Data.Output.CreateView(m_Data.ViewName));

        //

        auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(3);

        auto SourceResource       = Storage.GetResource(m_Data.Source).Get().get();
        auto IntermediateResource = Storage.GetResource(Intermediate).Get().get();
        auto OutputResource       = Storage.GetResource(m_Data.Output).Get().get();

        {
            std::array SrcInfo{
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 }
            };

            RHI::CpuDescriptorHandle
                &Source = SrcInfo[0].Descriptor,
                &Tmp    = SrcInfo[1].Descriptor,
                &Output = SrcInfo[2].Descriptor;

            Storage.GetResourceView(SourceView, &Source);
            Storage.GetResourceView(IntermediateOutputView, &Tmp);
            Storage.GetResourceView(OutputView, &Output);

            Descriptor->Copy(Descriptor.Offset, SrcInfo);
        }

        auto RootSignature = RHI::IRootSignature::Get(RHI::RSCommon::Type::BlurPass);

        CommandList.SetRootSignature(RootSignature);
        CommandList.SetConstants(uint32_t(BlurPassRS::BlurParams), m_GaussWeights.data(), m_GaussWeights.size());
        CommandList.SetDescriptorTable(uint32_t(BlurPassRS::InputOutput_TextureMap), Descriptor.GetGpuHandle());

        auto Size = Storage.GetResourceSize(m_Data.Source);

        uint32_t SourceOutput[2];
        SourceOutput[0] = 0;
        SourceOutput[1] = 1;

        uint32_t Iterations = m_Iterations;
        do
        {
            CommandList.SetPipelineState(m_BlurSubPassPipelineStateH);
            CommandList.SetConstants(uint32_t(BlurPassRS::IndexOffset), SourceOutput, std::size(SourceOutput));
            CommandList.Dispatch(Size.x, Size.y);

            if (Iterations == 1)
            {
                SourceOutput[0] = 1;
                SourceOutput[1] = 2;
            }
            else
            {
                SourceOutput[1] = 0;
                SourceOutput[0] = 1;
            }

            CommandList.InsertUAVBarrier(IntermediateResource);

            CommandList.SetPipelineState(m_BlurSubPassPipelineStateV);
            CommandList.SetConstants(uint32_t(BlurPassRS::IndexOffset), SourceOutput, std::size(SourceOutput));
            CommandList.Dispatch(Size.x, Size.y);

            if (Iterations != 1)
            {
                SourceOutput[0] = 0;
                SourceOutput[1] = 1;
                CommandList.InsertUAVBarrier(SourceResource);
            }
        } while (--Iterations > 0);
    }
} // namespace Neon::RG
