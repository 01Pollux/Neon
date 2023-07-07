#pragma once

#include <Private/RHI/Dx12/Resource/Resource.hpp>

namespace Neon::RHI
{
    class Dx12Swapchain;

    class TextureLoader
    {
    public:
        /// <summary>
        /// Load texture from memory as DDS.
        /// </summary>
        [[nodiscard]] static TextureLoader LoadDDS(
            const uint8_t* Data,
            size_t         DataSize);

        /// <summary>
        /// Load texture from memory as WIC.
        /// </summary>
        [[nodiscard]] static TextureLoader LoadWIC(
            const uint8_t* Data,
            size_t         DataSize);

        /// <summary>
        /// Get loaded texture.
        /// </summary>
        [[nodiscard]] Dx12Texture* Release() noexcept;

        /// <summary>
        /// Get upload id for texture.
        /// </summary>
        [[nodiscard]] uint64_t GetUploadId() const noexcept;

    private:
        TextureLoader(
            Win32::ComPtr<ID3D12Resource>      Texture,
            Win32::ComPtr<D3D12MA::Allocation> Allocation,
            std::span<const SubresourceDesc>   Subresources);

    private:
        uint64_t          m_UploadId;
        UPtr<Dx12Texture> m_Texture;
    };
} // namespace Neon::RHI
