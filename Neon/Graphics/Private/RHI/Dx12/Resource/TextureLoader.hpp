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
            const uint8_t*             Data,
            size_t                     DataSize,
            const RHI::MResourceState& InitialState);

        /// <summary>
        /// Load texture from memory as WIC.
        /// </summary>
        [[nodiscard]] static TextureLoader LoadWIC(
            const uint8_t*             Data,
            size_t                     DataSize,
            const RHI::MResourceState& InitialState);

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
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
            std::span<const SubresourceDesc>    Subresources,
            const RHI::MResourceState&          InitialState);

    private:
        uint64_t          m_UploadId;
        UPtr<Dx12Texture> m_Texture;
    };
} // namespace Neon::RHI
