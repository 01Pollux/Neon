#pragma once

#include <RHI/Swapchain.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/Commands/CommandAllocator.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>

namespace Neon::RHI
{
    class Dx12Swapchain final : public ISwapchain
    {
        static constexpr DXGI_FORMAT SwapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    public:
        Dx12Swapchain(
            const InitDesc& Desc);

        ~Dx12Swapchain() override;

        void PrepareFrame() override;

        void Present() override;

        void Resize(
            const Size2I& Size) override;

    private:
        void TestCommandList();

    private:
        Win32::ComPtr<IDXGISwapChain3> m_Swapchain;

        UPtr<Dx12CommandQueue> m_CommandQueue;

        std::vector<UPtr<Dx12CommandAllocator>> m_CommandAllocators;

        UPtr<Dx12CommandList> m_CommandList;

        std::map<ID3D12Resource*, D3D12_CPU_DESCRIPTOR_HANDLE> m_RenderTargets;
    };
} // namespace Neon::RHI