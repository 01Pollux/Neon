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
        static constexpr uint32_t BlurGaussWeightCount = 9;

        using GaussWeightsList = std::array<float, BlurGaussWeightCount>;

    public:
        struct BlurPassData
        {
            StringU8 ViewName;

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

        /// <summary>
        /// Set the number of iterations for the gaussian blur.
        /// </summary>
        void SetIterations(
            uint32_t Iterations);

        /// <summary>
        /// Get the number of iterations for the gaussian blur.
        /// </summary>
        uint32_t GetIterations() const noexcept;

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        BlurPassData m_Data;

        Ptr<RHI::IPipelineState> m_BlurSubPassPipelineStateH, m_BlurSubPassPipelineStateV;

        float            m_Sigma;
        GaussWeightsList m_GaussWeights;

        uint32_t m_Iterations = 1;
    };
} // namespace Neon::RG