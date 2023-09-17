#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>
#include <Renderer/Material/Material.hpp>

namespace Neon::RG
{
    class SSAOPass : public ComputeRenderPass<SSAOPass>
    {
        friend class RenderPass;

        struct DataType
        {
            static constexpr size_t DescriptorsCount = 3;

            ResourceViewId NormalMap;
            ResourceViewId DepthMap;
            ResourceViewId SSAOOutput;
        };

        struct ParamsType
        {
            float Radius           = 1.f;
            float Bias             = 0.01f;
            float Magnitude        = 1.5f;
            float Contrast         = 1.5f;
            float ResolutionFactor = 1.0f;
        };

    public:
        SSAOPass(
            uint32_t SampleCount = 8);

        /// <summary>
        /// Add the SSAO pass to the graph.
        /// </summary>
        static void AddPass(
            GraphBuilder& Builder);

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
        void PreDispatch(
            const GraphStorage&);

        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        /// <summary>
        /// Get the SSAO parameters.
        /// </summary>
        [[nodiscard]] const ParamsType* GetSSAOParams() const
        {
            return std::launder(std::bit_cast<ParamsType*>(m_Params.get()));
        }

        /// <summary>
        /// Get the SSAO parameters.
        /// </summary>
        [[nodiscard]] ParamsType* GetSSAOParams() noexcept
        {
            return std::launder(std::bit_cast<ParamsType*>(m_Params.get()));
        }

        /// <summary>
        /// Get the SSAO samples.
        /// </summary>
        [[nodiscard]] Vector4* GetSSAOSamples()
        {
            return std::launder(std::bit_cast<Vector4*>(std::bit_cast<uint8_t*>(m_Params.get()) + sizeof(ParamsType)));
        }

        /// <summary>
        /// Get the SSAO params size.
        /// </summary>
        [[nodiscard]] size_t GetParamsSize() const noexcept
        {
            return sizeof(ParamsType) + sizeof(Vector4) * m_SampleCount;
        }

    private:
        RHI::SSyncTexture m_NoiseTexture;

        Ptr<RHI::IRootSignature> m_SSAORootSignature;
        Ptr<RHI::IPipelineState> m_SSAOPipeline;

        /// <summary>
        /// Raw blob containing the SSAO parameters.
        /// Layout:
        /// sizeof(ParamsType) + sizeof(Vector4) * SampleCount
        /// </summary>
        std::unique_ptr<uint8_t[]> m_Params;

        size_t m_SampleCount = 0;

        DataType m_Data;
    };
} // namespace Neon::RG