#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>
#include <RenderGraph/Passes/BlurPass.hpp>

#include <RHI/RootSignature.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/GlobalDescriptors.hpp>

#include <Asset/Manager.hpp>
#include <Asset/Types/Shader.hpp>

#include <Input/System.hpp>

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
    class BlurSubPass : public ComputeRenderPass<BlurSubPass>
    {
        friend class RenderPass;

        /// <summary>
        /// Kernel size in compute shader.
        /// </summary>
        static constexpr uint32_t KernelSize           = 16;
        static constexpr uint32_t BlurGaussWeightCount = 9;

        using GaussWeightsList = std::array<float, BlurGaussWeightCount>;

    public:
        BlurSubPass(
            BlurPass::BlurPassData Data,
            bool                   IsHorizontal);

    public:
        /// <summary>
        /// Set the sigma value for the gaussian blur.
        /// </summary>
        void SetSigma(
            float Sigma);

        /// <summary>
        /// Get the sigma value for the gaussian blur.
        /// </summary>
        float GetSigma() const noexcept;

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        BlurPass::BlurPassData m_Data;

        Ptr<RHI::IRootSignature> m_BlurSubPassRootSignature;
        Ptr<RHI::IPipelineState> m_BlurSubPassPipelineState;

        float            m_Sigma;
        GaussWeightsList m_GaussWeights;

        uint32_t m_Iterations = 1;
        bool     m_IsHorizontal;
    };

    //

    BlurSubPass::BlurSubPass(
        BlurPass::BlurPassData Data,
        bool                   IsHorizontal) :
        RenderPass("BlurSubPass"),
        m_Data(std::move(Data)),
        m_IsHorizontal(IsHorizontal)
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

        m_BlurSubPassRootSignature =
            RHI::RootSignatureBuilder()
                .Add32BitConstants<GaussWeightsList>("c_BlurParams", 0, 1)
                .AddDescriptorTable(
                    RHI::RootDescriptorTable()
                        .AddSrvRange("c_Input", 0, 1, 1)
                        .AddUavRange("c_Output", 0, 1, 1))
                .ComputeOnly()
                .Build();

        if (m_IsHorizontal)
        {
            ShaderDesc.Macros.Append(STR("BLUR_H"));
        }

        m_BlurSubPassPipelineState =
            RHI::PipelineStateBuilderC{
                .RootSignature = m_BlurSubPassRootSignature,
                .ComputeShader = Shader->LoadShader(ShaderDesc)
            }
                .Build();
    }

    //

    void BlurSubPass::SetSigma(
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

        m_GaussWeights = { 0.0002,
                           0.0060,
                           0.0606,
                           0.2417,
                           0.3829,
                           0.2417,
                           0.0606,
                           0.0060,
                           0.0002 };
    }

    float BlurSubPass::GetSigma() const noexcept
    {
        return m_Sigma;
    }

    //

    void BlurSubPass::ResolveResources(
        ResourceResolver& Resolver)
    {
        const ResourceId Intermediate(STR("BlurSubPassIntermediate_") + m_Data.ViewName);

        auto [Desc, Flags] = Resolver.GetResourceDescAndFlags(m_Data.Source);
        if (m_IsHorizontal)
        {
            const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
            const ResourceViewId IntermediateView(Intermediate.CreateView(STR("O_") + m_Data.ViewName));

            Resolver.CreateTexture(
                Intermediate,
                *Desc,
                Flags);

            Resolver.ReadTexture(SourceView, ResourceReadAccess::NonPixelShader, m_Data.SourceDesc);
            Resolver.WriteResource(IntermediateView);
        }
        else
        {
            const ResourceViewId IntermediateView(Intermediate.CreateView(m_Data.ViewName));
            const ResourceViewId OutputView(m_Data.Output.CreateView(STR("I_") + m_Data.ViewName));

            Resolver.CreateTexture(
                m_Data.Output,
                *Desc,
                Flags);

            Resolver.ReadTexture(IntermediateView, ResourceReadAccess::NonPixelShader, m_Data.SourceDesc);
            Resolver.WriteResource(OutputView, m_Data.OutputDesc);
        }
    }

    void BlurSubPass::DispatchTyped(
        const GraphStorage&     Storage,
        RHI::ComputeCommandList CommandList)
    {
        static bool Disabled = false;
        if (Input::IsKeyPressed(Input::EKeyboardInput::K))
        {
            Disabled = !Disabled;
        }
        if (Disabled)
        {
            return;
        }

        const ResourceId Intermediate(STR("BlurSubPassIntermediate_") + m_Data.ViewName);

        //
        // We will allocate 4 descriptors, one for input and one for output for each stage
        auto Descriptor = RHI::IFrameDescriptorHeap::Get(RHI::DescriptorType::ResourceView)->Allocate(2);

        {
            std::array SrcInfo{
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 },
                RHI::IDescriptorHeap::CopyInfo{ .CopySize = 1 }
            };

            RHI::CpuDescriptorHandle
                &Input  = SrcInfo[0].Descriptor,
                &Output = SrcInfo[1].Descriptor;

            if (m_IsHorizontal)
            {
                const ResourceViewId SourceView(m_Data.Source.CreateView(m_Data.ViewName));
                const ResourceViewId IntermediateView(Intermediate.CreateView(STR("O_") + m_Data.ViewName));

                Storage.GetResourceView(SourceView, &Input);
                Storage.GetResourceView(IntermediateView, &Output);
            }
            else
            {
                const ResourceViewId IntermediateView(Intermediate.CreateView(m_Data.ViewName));
                const ResourceViewId OutputView(m_Data.Output.CreateView(STR("I_") + m_Data.ViewName));

                Storage.GetResourceView(IntermediateView, &Input);
                Storage.GetResourceView(OutputView, &Output);
            }

            Descriptor->Copy(Descriptor.Offset, SrcInfo);
        }

        CommandList.SetRootSignature(m_BlurSubPassRootSignature);
        CommandList.SetConstants(0, m_GaussWeights);

        auto Size = Storage.GetResourceSize(m_Data.Source);

        for (uint32_t i = 0; i < m_Iterations; i++)
        {
            CommandList.SetPipelineState(m_BlurSubPassPipelineState);
            CommandList.SetDescriptorTable(1, Descriptor.GetGpuHandle());

            if (m_IsHorizontal)
            {
                CommandList.Dispatch(Math::DivideByMultiple(Size.x, KernelSize), Size.y);
            }
            else
            {
                CommandList.Dispatch(Size.x, Math::DivideByMultiple(Size.y, KernelSize));
            }
        }
    }
} // namespace Neon::RG

namespace Neon::RG
{
    void BlurPass::AddPass(
        GraphBuilder& Builder,
        BlurPassData  Data)
    {
        Builder.AddPass<BlurSubPass>(Data, true);
        Builder.AddPass<BlurSubPass>(std::move(Data), false);
    }
} // namespace Neon::RG