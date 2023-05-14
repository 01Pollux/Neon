#pragma once

#include <RHI/Swapchain.hpp>
#include <RHI/Fence.hpp>

#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>

#include <Private/RHI/Dx12/Budget.hpp>

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

    public:
        /// <summary>
        /// Allocate or reuse command lists
        /// </summary>
        [[nodiscard]] std::vector<ICommandList*> AllocateCommandLists(
            D3D12_COMMAND_LIST_TYPE Type,
            size_t                  Count);

        /// <summary>
        /// Free command lists.
        /// </summary>
        void FreeCommandLists(
            D3D12_COMMAND_LIST_TYPE  Type,
            std::span<ICommandList*> Commands);

        /// <summary>
        /// Reset command lists with new command allocator.
        /// </summary>
        void ResetCommandLists(
            D3D12_COMMAND_LIST_TYPE  Type,
            std::span<ICommandList*> Commands);

        [[nodiscard]] ICommandQueue* GetQueue(
            CommandQueueType Type) override;

    private:
        /// <summary>
        /// Create the swapchain.
        /// </summary>
        void
        CreateSwapchain(
            const InitDesc& Desc);

        void ResizeBackbuffers(
            size_t NewSize);

    private:
        Win32::ComPtr<IDXGISwapChain3> m_Swapchain;
        std::vector<Dx12Texture>       m_BackBuffers;

        BudgetManager                            m_BudgetManager;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RenderTargets;
    };
} // namespace Neon::RHI