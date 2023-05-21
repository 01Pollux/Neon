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
            const PipelineStateBuilder& Builder);

        /// <summary>
        /// Get or save pipeline state from cache
        /// </summary>
        [[nodiscard]] static Ptr<IPipelineState> Load(
            const ComputePipelineStateBuilder& Builder);

    private:
        struct BuildResult
        {
            SHA256::Bytes                                     Digest;
            std::list<std::vector<CD3DX12_DESCRIPTOR_RANGE1>> RangesList;
            std::vector<CD3DX12_ROOT_PARAMETER1>              Parameters;
            std::vector<CD3DX12_STATIC_SAMPLER_DESC>          StaticSamplers;
            D3D12_ROOT_SIGNATURE_FLAGS                        Flags;
        };

        /// <summary
        /// Get or save root signature from cache
        /// </summary>
        [[nodiscard]] static BuildResult Build(
            const RootSignatureBuilder& Builder);
    };
} // namespace Neon::RHI