#pragma once

#include <RHI/Swapchain.hpp>
#include <RHI/Fence.hpp>

#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>

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
        /// <summary>
        /// Create the swapchain.
        /// </summary>
        void CreateSwapchain(
            const InitDesc& Desc);

        void ResizeBackbuffers(
            size_t NewSize);

    private:
        Win32::ComPtr<IDXGISwapChain3> m_Swapchain;

        UPtr<Dx12CommandQueue>   m_CommandQueue;
        std::vector<Dx12Texture> m_BackBuffers;

        uint64_t     m_FenceValue = 0;
        UPtr<IFence> m_FrameFence;

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RenderTargets;
    };
} // namespace Neon::RHI