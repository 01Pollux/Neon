#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>
#include <Renderer/Material/Material.hpp>

namespace Neon::RG
{
    class SSAOPass : public ComputeRenderPass<SSAOPass>
    {
        friend class RenderPass;

        static constexpr size_t SampleCount = 14;

        struct ParamsType
        {
            Vector4 Samples[SampleCount];
            float   Radius    = 1.f;
            float   Bias      = 0.01f;
            float   Magnitude = 1.5f;
            float   Contrast  = 1.5f;
        };

    public:
        SSAOPass();

        /// <summary>
        /// Add the SSAO pass to the graph.
        /// </summary>
        static void AddPass(
            GraphBuilder& Builder);

    public:
        /// <summary>
        /// Regenerate the SSAO samples.
        /// </summary>
        void RegenerateSamples();

        /// <summary>
        /// Get the SSAO radius.
        /// </summary>
        [[nodiscard]] float GetRadius() const noexcept;

        /// <summary>
        /// Set the SSAO radius.
        /// </summary>
        void SetRadius(
            float Val) noexcept;

        /// <summary>
        /// Get the SSAO bias.
        /// </summary>
        [[nodiscard]] float GetBias() const noexcept;

        /// <summary>
        /// Set the SSAO bias.
        /// </summary>
        void SetBias(
            float Val) noexcept;

        /// <summary>
        /// Get the SSAO magnitude.
        /// </summary>
        [[nodiscard]] float GetMagnitude() const noexcept;

        /// <summary>
        /// Set the SSAO magnitude.
        /// </summary>
        void SetMagniute(
            float Val) noexcept;

        /// <summary>
        /// Get the SSAO contrast.
        /// </summary>
        [[nodiscard]] float GetContrast() const noexcept;

        /// <summary>
        /// Set the SSAO contrast.
        /// </summary>
        void SetContrast(
            float Val) noexcept;

    private:
        /// <summary>
        /// Generate the SSAO samples.
        /// </summary>
        void GenerateSamples();

        /// <summary>
        /// Generate the SSAO noise.
        /// </summary>
        void GenerateNoise();

    protected:
        void PreDispatch(
            const GraphStorage&);

        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        RHI::SSyncGpuResource m_NoiseTexture;

        Ptr<RHI::IRootSignature> m_SSAORootSignature;
        Ptr<RHI::IPipelineState> m_SSAOPipeline;

        ParamsType m_Params;
    };
} // namespace Neon::RG