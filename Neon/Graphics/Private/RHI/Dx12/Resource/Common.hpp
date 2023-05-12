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
} // namespace Neon::RHI