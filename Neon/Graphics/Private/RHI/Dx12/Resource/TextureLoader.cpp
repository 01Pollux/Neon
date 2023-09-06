#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/TextureLoader.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

namespace Neon::RHI
{
    Dx12Texture* TextureLoader::Release() noexcept
    {
        return m_Texture.release();
    }

    uint64_t TextureLoader::GetUploadId() const noexcept
    {
        return m_UploadId;
    }

    TextureLoader::TextureLoader(
        WinAPI::ComPtr<ID3D12Resource>      Texture,
        WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
        std::span<const SubresourceDesc>    Subresources) :
        m_Texture(std::make_unique<Dx12Texture>(std::move(Texture), std::move(Allocation), Subresources, m_UploadId))
    {
    }
} // namespace Neon::RHI
