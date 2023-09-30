#pragma once
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <RHI/PipelineState.hpp>
#include <Crypto/Sha256.hpp>

namespace Neon::RHI
{
    class Dx12PipelineState final : public IPipelineState
    {
    public:
        Dx12PipelineState(
            const PipelineStateBuilderG&              Builder,
            const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GraphicsDesc);

        Dx12PipelineState(
            const PipelineStateBuilderC&             Builder,
            const D3D12_COMPUTE_PIPELINE_STATE_DESC& ComputeDesc);

        /// <summary>
        /// Get underlying D3D12 pipeline state.
        /// </summary>
        [[nodiscard]] ID3D12PipelineState* Get();

        /// <summary>
        /// Get group size of compute shader
        /// </summary>
        [[nodiscard]] const Vector3U& GetComputeGroupSize() const override;

    private:
        WinAPI::ComPtr<ID3D12PipelineState> m_PipelineState;

        Vector3U m_ComputeGroupSize{};
    };

    class Dx12PipelineStateCache
    {
    public:
        /// <summary>
        /// Clear the pipeline state cache and release all cached pipeline states
        /// </summary>
        static void Flush();

        /// <summary>
        /// Get or save pipeline state from cache
        /// </summary>
        [[nodiscard]] static Ptr<IPipelineState> Load(
            const PipelineStateBuilderG& Builder);

        /// <summary>
        /// Get or save pipeline state from cache
        /// </summary>
        [[nodiscard]] static Ptr<IPipelineState> Load(
            const PipelineStateBuilderC& Builder);

    private:
        struct GraphicsBuildResult
        {
            Crypto::Sha256::Bytes                 Digest;
            D3D12_GRAPHICS_PIPELINE_STATE_DESC    Desc{};
            std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements;
            ShaderInputLayout                     InputLayout;

            GraphicsBuildResult() = default;
            NEON_CLASS_NO_COPY(GraphicsBuildResult);
            NEON_CLASS_MOVE(GraphicsBuildResult);
            ~GraphicsBuildResult() = default;
        };
        struct ComputeBuildResult
        {
            Crypto::Sha256::Bytes             Digest;
            D3D12_COMPUTE_PIPELINE_STATE_DESC Desc{};

            ComputeBuildResult() = default;
            NEON_CLASS_NO_COPY(ComputeBuildResult);
            NEON_CLASS_MOVE(ComputeBuildResult);
            ~ComputeBuildResult() = default;
        };

        /// <summary
        /// Build the pipeline state
        /// </summary>
        [[nodiscard]] static GraphicsBuildResult Build(
            const PipelineStateBuilderG& Builder);

        /// <summary
        /// Build the pipeline state
        /// </summary>
        [[nodiscard]] static ComputeBuildResult Build(
            const PipelineStateBuilderC& Builder);
    };
} // namespace Neon::RHI