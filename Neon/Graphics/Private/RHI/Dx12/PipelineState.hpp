#pragma once
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <RHI/PipelineState.hpp>
#include <Core/SHA256.hpp>

namespace Neon::RHI
{
    class Dx12PipelineState final : public IPipelineState
    {
    public:
        Dx12PipelineState(
            const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GraphicsDesc);

        Dx12PipelineState(
            const D3D12_COMPUTE_PIPELINE_STATE_DESC& ComputeDesc);

    private:
        Win32::ComPtr<ID3D12PipelineState> m_PipelineState;
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
            const PipelineStateBuilder<false>& Builder);

        /// <summary>
        /// Get or save pipeline state from cache
        /// </summary>
        [[nodiscard]] static Ptr<IPipelineState> Load(
            const PipelineStateBuilder<true>& Builder);

    private:
        struct GraphicsBuildResult
        {
            SHA256::Bytes                         Digest;
            D3D12_GRAPHICS_PIPELINE_STATE_DESC    Desc{};
            std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements;

            GraphicsBuildResult() = default;
            NEON_CLASS_NO_COPY(GraphicsBuildResult);
            NEON_CLASS_MOVE(GraphicsBuildResult);
            ~GraphicsBuildResult() = default;
        };
        struct ComputeBuildResult
        {
            SHA256::Bytes                     Digest;
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
            const PipelineStateBuilder<false>& Builder);

        /// <summary
        /// Build the pipeline state
        /// </summary>
        [[nodiscard]] static ComputeBuildResult Build(
            const PipelineStateBuilder<true>& Builder);
    };
} // namespace Neon::RHI