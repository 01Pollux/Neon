#pragma once

#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12DeviceFeatures
    {
        friend class Dx12RenderDevice;

    public:
        /// <summary>
        /// Get root signature version.
        /// </summary>
        [[nodiscard]] D3D_ROOT_SIGNATURE_VERSION GetRootSignatureVersion() const;

        /// <summary>
        /// Get max view descriptor heap size for sampler or non-sampler.
        /// </summary>
        [[nodiscard]] uint32_t MaxDescriptorHeapSize(
            bool Sampler) const;

    private:
        void Initialize(
            ID3D12Device* Device);

    private:
        D3D_ROOT_SIGNATURE_VERSION m_RootSignatureVersion;
        uint32_t                   m_DescriptorHeapSize[2];
    };
} // namespace Neon::RHI