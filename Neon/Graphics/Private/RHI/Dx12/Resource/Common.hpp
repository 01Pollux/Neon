#pragma once

#include <RHI/Resource/Common.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12GpuResource;

    /// <summary>
    /// Get D3D12 resource from Neon resource.
    /// </summary>
    ID3D12Resource* GetDx12Resource(
        IGpuResource* Resource);

    //

    /// <summary>
    /// Convert Neon resource state to D3D12 resource states.
    /// </summary>
    D3D12_RESOURCE_STATES CastResourceStates(
        EResourceState States);

    /// <summary>
    /// Convert Neon resource states to D3D12 resource states.
    /// </summary>
    D3D12_RESOURCE_STATES CastResourceStates(
        const MResourceState& States);

    /// <summary>
    /// Convert D3D12 resource states to Neon resource states.
    /// </summary>
    MResourceState CastResourceStates(
        D3D12_RESOURCE_STATES States);

    //

    /// <summary>
    /// Convert Neon resource format to D3D12 resource dimension.
    /// </summary>
    DXGI_FORMAT CastFormat(
        EResourceFormat Format) noexcept;

    /// <summary>
    /// Convert D3D12 format to Neon resource format.
    /// </summary>
    EResourceFormat CastFormat(
        DXGI_FORMAT Format) noexcept;

    //

    /// <summary>
    /// Convert Neon sampler filter to D3D12 sampler filter.
    /// </summary>
    D3D12_FILTER CastFilter(
        ESamplerFilter Filter) noexcept;

    /// <summary>
    /// Convert D3D12 sampler filter to Neon sampler filter.
    /// </summary>
    ESamplerFilter CastFilter(
        D3D12_FILTER Filter) noexcept;

    //

    /// <summary>
    /// Convert Neon texture address mode to D3D12 texture address mode.
    /// </summary>
    D3D12_TEXTURE_ADDRESS_MODE CastAddressMode(
        ESamplerMode AddressMode) noexcept;

    /// <summary>
    /// Convert D3D12 texture address mode to Neon sampler filter.
    /// </summary>
    ESamplerMode CastAddressMode(
        D3D12_TEXTURE_ADDRESS_MODE AddressMode) noexcept;

    //

    /// <summary>
    /// Convert Neon sampler comparison function to D3D12 comparison function.
    /// </summary>
    D3D12_COMPARISON_FUNC CastComparisonFunc(
        ESamplerCmp CmpFunc) noexcept;

    /// <summary>
    /// Convert D3D12 comparison function to Neon sampler comparison function.
    /// </summary>
    ESamplerMode CastComparisonFunc(
        D3D12_TEXTURE_ADDRESS_MODE AddressMode) noexcept;

} // namespace Neon::RHI