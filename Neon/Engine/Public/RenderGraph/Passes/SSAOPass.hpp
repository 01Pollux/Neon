#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class SSAOPass : public ComputeRenderPass<SSAOPass>
    {
        friend class RenderPass;

        struct Params
        {
            std::vector<Vector4> Samples;

            float Radius           = 1.f;
            float Bias             = 0.01f;
            float Magnitude        = 1.5f;
            float Contrast         = 1.5f;
            float ResolutionFactor = 0.5f;
        };

    public:
        SSAOPass(
            uint32_t SampleCount = 16);

    public:
        /// <summary>
        /// Regenerate the SSAO samples.
        /// If SampleCount is 0, the current sample count will be used.
        /// </summary>
        void RegenerateSamples(
            uint32_t SampleCount = 0);

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

        /// <summary>
        /// Get the SSAO resolution factor.
        /// </summary>
        [[nodiscard]] float GetResolutionFactor() const noexcept;

        /// <summary>
        /// Set the SSAO resolution factor.
        /// </summary>
        void SetResolutionFactor(
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
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&       Storage,
            RHI::IComputeCommandList* CommandList);

    private:
        RHI::USyncTexture m_NoiseTexture;

        Ptr<Renderer::IMaterial> m_Material;
        Params                   m_Params;
    };
} // namespace Neon::RG