#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::Renderer
{
    class IMaterial;
}

namespace Neon::RG
{
    class BlurPass : public ComputeRenderPass<BlurPass>
    {
        friend class RenderPass;

        /// <summary>
        /// Kernel size in compute shader.
        /// </summary>
        static constexpr uint32_t KernelSize           = 16;
        static constexpr uint32_t BlurGaussWeightCount = 11;

        using GaussWeightsList = std::array<float, BlurGaussWeightCount>;

    public:
        struct BlurPassData
        {
            String ViewName;

            ResourceId Source;
            ResourceId Output;

            RHI::SRVDescOpt SourceDesc;
            RHI::UAVDescOpt OutputDesc;
        };

        BlurPass(
            BlurPassData Data);

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
        BlurPassData m_Data;

        Ptr<RHI::IRootSignature> m_BlurPassRootSignature;
        Ptr<RHI::IPipelineState>
            m_BlurPassPipelineStateH,
            m_BlurPassPipelineStateV;

        float            m_Sigma;
        GaussWeightsList m_GaussWeights;

        uint32_t m_Iterations = 1;
    };
} // namespace Neon::RG