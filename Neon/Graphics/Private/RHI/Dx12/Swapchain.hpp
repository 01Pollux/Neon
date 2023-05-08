#pragma once

#include <RHI/Swapchain.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12Swapchain final : public ISwapchain
    {
        static constexpr DXGI_FORMAT SwapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    public:
        Dx12Swapchain(
            const InitDesc& Desc);

        void PrepareFrame() override;

        void Present() override;

        const Size2I& GetSize() override;

        void Resize(
            const Size2I& Size) override;

    private:
        Win32::ComPtr<IDXGISwapChain> m_Swapchain;
    };
} // namespace Neon::RHI