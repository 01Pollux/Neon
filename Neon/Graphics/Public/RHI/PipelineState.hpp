#pragma once

#include <Core/Neon.hpp>

namespace Neon::RHI
{
    /*class PipelineStateBuilder
    {
        public:
        PipelineStateBuilder() = default;

        PipelineStateBuilder& SetVertexShader(
            const std::string& Path);

        PipelineStateBuilder& SetPixelShader(
            const std::string& Path);

        PipelineStateBuilder& SetVertexLayout(
            const std::vector<VertexElement>& Elements);

        PipelineStateBuilder& SetDepthStencilState(
            const DepthStencilState& State);

        PipelineStateBuilder& SetRasterizerState(
            const RasterizerState& State);

        PipelineStateBuilder& SetBlendState(
            const BlendState& State);

        PipelineStateBuilder& SetPrimitiveTopology(
            D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology);

        PipelineStateBuilder& SetRenderTargetFormats(
            const std::vector<DXGI_FORMAT>& Formats);

        PipelineStateBuilder& SetDepthStencilFormat(
            DXGI_FORMAT Format);

        PipelineStateBuilder& SetSampleDesc(
            const DXGI_SAMPLE_DESC& SampleDesc);

        PipelineStateBuilder& SetSampleMask(
            UINT SampleMask);

        PipelineStateBuilder& SetNodeMask(
            UINT NodeMask);

        PipelineStateBuilder& SetCachedPSO(
            const D3D12_CACHED_PIPELINE_STATE& CachedPSO);

        PipelineStateBuilder& SetFlags(
            D3D12_PIPELINE_STATE_FLAGS Flags);

        PipelineStateBuilder& SetInputLayout(
            const D3D12_INPUT_LAYOUT_DESC& InputLayout);

        PipelineStateBuilder& SetIBStripCutValue(
            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue);

        PipelineStateBuilder& SetPrimitiveRestartEnable(
            bool PrimitiveRestartEnable);

        PipelineStateBuilder& SetDepthClipEnable(
            bool DepthClipEnable);

        PipelineStateBuilder& SetMultisampleEnable(
            bool MultisampleEnable);

        PipelineStateBuilder& SetAntialiasedLineEnable(
            bool AntialiasedLineEnable);

        PipelineStateBuilder& SetForcedSampleCount(
            UINT ForcedSampleCount);

        PipelineStateBuilder& SetConservativeRaster(
            D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster);

        PipelineStateBuilder& SetDepthBias(
            INT DepthBias);

        PipelineStateBuilder& SetDepthBiasClamp(
            FLOAT DepthBiasClamp);

        PipelineStateBuilder& SetSlopeScaledDepthBias(
            FLOAT SlopeScaledDepthBias);

        PipelineStateBuilder& SetDepthBoundsTestEnable(
            bool DepthBoundsTestEnable);

        Pipeline
    };>*/

    class IPipelineState
    {
    public:
        virtual ~IPipelineState() = default;
    };
} // namespace Neon::RHI