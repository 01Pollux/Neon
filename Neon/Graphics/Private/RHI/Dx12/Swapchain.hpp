#pragma once

#include <RHI/Device.hpp>
#include <RHI/Swapchain.hpp>
#include <RHI/Fence.hpp>

#include <Private/RHI/Dx12/Resource/GraphicsMemoryAllocator.hpp>
#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/FrameManager.hpp>

#include <RHI/Resource/Views/RenderTarget.hpp>

namespace Neon::RHI
{
    class Dx12Swapchain final : public ISwapchain
    {
        using DescriptorHeapAllocators = std::array<Dx12DescriptorHeapBuddyAllocator, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES>;

    public:
        Dx12Swapchain(
            const SwapchainCreateDesc& Desc);

        void PrepareFrame() override;

        void Present() override;

        const Size2I& GetSize() override;

        EResourceFormat GetFormat() override;

        IGpuResource* GetBackBuffer() override;

        CpuDescriptorHandle GetBackBufferView() override;

        void Resize(
            const Size2I&   Size,
            EResourceFormat NewFormat) override;

        [[nodiscard]] ICommandQueue* GetQueue(
            CommandQueueType Type) override;

        [[nodiscard]] IDescriptorHeapAllocator* GetDescriptorHeapManager(
            DescriptorType Type,
            bool           Dynamic) override;

    public:
        /// <summary>
        /// Get the singleton instance.
        /// </summary>
        static [[nodiscard]] Dx12Swapchain* Get();

        /// <summary>
        /// Initialize the swapchain.
        /// </summary>
        void PostInitialize(
            const SwapchainCreateDesc& Desc);

        /// <summary>
        /// Shutdown the swapchain.
        /// </summary>
        void Shutdown();

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

        void SafeRelease(
            IDescriptorHeapAllocator*   Allocator,
            const DescriptorHeapHandle& Handle) override;

        /// <summary>
        /// Enqueue buffer to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Dx12Buffer::Handle& Handle);

        /// <summary>
        /// Enqueue descriptor to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Win32::ComPtr<ID3D12DescriptorHeap>& Resource);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Win32::ComPtr<ID3D12Resource>&      Resource,
            const Win32::ComPtr<D3D12MA::Allocation>& Allocation);

        /// <summary>
        /// Wait for a copy command list to be executed.
        /// </summary>
        void WaitForCopy(
            ICommandQueue* Queue,
            uint64_t       FenceValue);

        /// <summary>
        /// Enqueue a copy command list to be executed.
        /// </summary>
        uint64_t EnqueueRequestCopy(
            std::function<void(ICopyCommandList*)> Task) override;

    private:
        /// <summary>
        /// Create the swapchain.
        /// </summary>
        void CreateSwapchain(
            const SwapchainCreateDesc& Desc);

        /// <summary>
        /// Resize the swapchain.
        /// </summary>
        void ResizeBackbuffers(
            uint32_t NewSize);

    private:
        Windowing::IWindowApp* m_WindowApp;
        Size2I                 m_Size;

        Win32::ComPtr<IDXGISwapChain3> m_Swapchain;
        UPtr<FrameManager>             m_BudgetManager;

        std::vector<Dx12Texture> m_BackBuffers;
        Views::RenderTarget      m_RenderTargets;

        EResourceFormat m_BackbufferFormat = EResourceFormat::Unknown;

        UPtr<DescriptorHeapAllocators> m_StaticDescriptorHeap;
        UPtr<DescriptorHeapAllocators> m_DynamicDescriptorHeap;
    };
} // namespace Neon::RHI