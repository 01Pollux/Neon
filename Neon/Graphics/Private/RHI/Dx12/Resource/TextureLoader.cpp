#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Resource/TextureLoader.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

namespace Neon::RHI
{
    Dx12Texture* TextureLoader::Release() noexcept
    {
        return m_Texture.release();
    }

    TextureLoader::TextureLoader(
        std::future<void>&                  CopyTask,
        WinAPI::ComPtr<ID3D12Resource>      Texture,
        WinAPI::ComPtr<D3D12MA::Allocation> Allocation,
        std::span<const SubresourceDesc>    Subresources,
        const wchar_t*                      Name,
        const RHI::MResourceState&          InitialState) :
        m_Texture(std::make_unique<Dx12Texture>(
            std::move(Texture),
            std::move(Allocation),
            Subresources,
            CopyTask,
            Name,
            InitialState))
    {
    }
} // namespace Neon::RHI
