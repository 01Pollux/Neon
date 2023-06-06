#pragma once

#include <RHI/Resource/Common.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12GpuResource;

    /// <summary>
    /// Get D3D12 resource from Neon resource.
    /// </summary>
    [[nodiscard]] ID3D12Resource* GetDx12Resource(
        IGpuResource* Resource);

    //

    /// <summary>
    /// Convert Neon resource state to D3D12 resource states.
    /// </summary>
    [[nodiscard]] D3D12_RESOURCE_STATES CastResourceStates(
        EResourceState States);

    /// <summary>
    /// Convert Neon resource states to D3D12 resource states.
    /// </summary>
    [[nodiscard]] D3D12_RESOURCE_STATES CastResourceStates(
        const MResourceState& States);

    /// <summary>
    /// Convert D3D12 resource states to Neon resource states.
    /// </summary>
    [[nodiscard]] MResourceState CastResourceStates(
        D3D12_RESOURCE_STATES States);

    //

    /// <summary>
    /// Convert Neon resource format to D3D12 resource dimension.
    /// </summary>
    [[nodiscard]] DXGI_FORMAT CastFormat(
        EResourceFormat Format);

    /// <summary>
    /// Convert D3D12 format to Neon resource format.
    /// </summary>
    [[nodiscard]] EResourceFormat CastFormat(
        DXGI_FORMAT Format);

    //

    /// <summary>
    /// Convert Neon sampler filter to D3D12 sampler filter.
    /// </summary>
    [[nodiscard]] D3D12_FILTER CastFilter(
        ESamplerFilter Filter);

    /// <summary>
    /// Convert D3D12 sampler filter to Neon sampler filter.
    /// </summary>
    [[nodiscard]] ESamplerFilter CastFilter(
        D3D12_FILTER Filter);

    //

    /// <summary>
    /// Convert Neon texture address mode to D3D12 texture address mode.
    /// </summary>
    [[nodiscard]] D3D12_TEXTURE_ADDRESS_MODE CastAddressMode(
        ESamplerMode AddressMode);

    /// <summary>
    /// Convert D3D12 texture address mode to Neon sampler filter.
    /// </summary>
    [[nodiscard]] ESamplerMode CastAddressMode(
        D3D12_TEXTURE_ADDRESS_MODE AddressMode);

    //

    /// <summary>
    /// Convert Neon sampler comparison function to D3D12 comparison function.
    /// </summary>
    [[nodiscard]] D3D12_COMPARISON_FUNC CastComparisonFunc(
        ECompareFunc CmpFunc);

    /// <summary>
    /// Convert D3D12 comparison function to Neon sampler comparison function.
    /// </summary>
    [[nodiscard]] ESamplerMode CastComparisonFunc(
        D3D12_TEXTURE_ADDRESS_MODE AddressMode);

    //

    /// <summary>
    /// Convert Neon resource flags to D3D12 resource flags.
    /// </summary>
    [[nodiscard]] MResourceFlags CastResourceFlags(
        D3D12_RESOURCE_FLAGS Flags);

    /// <summary>
    /// Convert Neon resource flags to D3D12 resource flags.
    /// </summary>
    [[nodiscard]] D3D12_RESOURCE_FLAGS CastResourceFlags(
        const MResourceFlags& Flags);

    //

    /// <summary>
    /// Convert Neon primitive topology to D3D12 primitive topology.
    /// </summary>
    [[nodiscard]] D3D_PRIMITIVE_TOPOLOGY CastPrimitiveTopology(
        PrimitiveTopology Topology);
} // namespace Neon::RHI