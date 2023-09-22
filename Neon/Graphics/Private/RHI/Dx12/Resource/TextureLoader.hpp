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
            const wchar_t*             Name,
            std::future<void>&         CopyTask,
            const RHI::MResourceState& InitialState);

        /// <summary>
        /// Load texture from memory as WIC.
        /// </summary>
        [[nodiscard]] static TextureLoader LoadWIC(
            const uint8_t*             Data,
            size_t                     DataSize,
            const wchar_t*             Name,
            std::future<void>&         CopyTask,
            const RHI::MResourceState& InitialState);

        /// <summary>
        /// Get loaded texture.
        /// </summary>
        [[nodiscard]] Dx12GpuResource* Release() noexcept;

    private:
        TextureLoader(
            std::future<void>&                  CopyTask,
            WinAPI::ComPtr<ID3D12Resource>      Texture,
            WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
            std::span<const SubresourceDesc>    Subresources,
            const wchar_t*                      Name,
            const RHI::MResourceState&          InitialState);

    private:
        UPtr<Dx12GpuResource> m_Texture;
    };
} // namespace Neon::RHI
