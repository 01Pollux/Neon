#pragma once

#include <RHI/Swapchain.hpp>
#include <RHI/Fence.hpp>

#include <Private/RHI/Dx12/Resource/GraphicsMemoryAllocator.hpp>
#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>
#include <RHI/Resource/Views/RenderTarget.hpp>
#include <Private/RHI/Dx12/Budget.hpp>

#include <random>

namespace Neon::RHI
{
    // TODO : remove

    class IRootSignature;
    class IPipelineState;

    //

    class Dx12Swapchain final : public ISwapchain
    {
    public:
        Dx12Swapchain(
            const InitDesc& Desc);

        ~Dx12Swapchain() override;

        void PrepareFrame() override;

        void Present() override;

        EResourceFormat GetFormat() override;

        void Resize(
            const Size2I&   Size,
            EResourceFormat NewFormat) override;

        [[nodiscard]] ICommandQueue* GetQueue(
            CommandQueueType Type) override;

        [[nodiscard]] IResourceStateManager* GetStateManager() override;

        [[nodiscard]] IDescriptorHeapAllocator* GetDescriptorHeapManager(
            DescriptorType Type,
            bool           Dynamic) override;

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

        /// <summary>
        /// Enqueue descriptor heap to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Ptr<IDescriptorHeap>& Heap) override;

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Ptr<IDescriptorHeapAllocator>& Allocator,
            const DescriptorHeapHandle&          Handle) override;

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Dx12Buffer::Handle& Handle);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Win32::ComPtr<ID3D12Resource>& Resource);

    public:
        /// <summary>
        /// Get memory allocator
        /// </summary>
        [[nodiscard]] GraphicsMemoryAllocator* GetAllocator();

    private:
        /// <summary>
        /// Create the swapchain.
        /// </summary>
        void CreateSwapchain(
            const InitDesc& Desc);

        /// <summary>
        /// Resize the swapchain.
        /// </summary>
        void ResizeBackbuffers(
            uint32_t NewSize);

    private:
        Windowing::IWindowApp*         m_WindowApp;
        Win32::ComPtr<IDXGISwapChain3> m_Swapchain;
        GraphicsMemoryAllocator        m_MemoryAllocator;
        BudgetManager                  m_BudgetManager;

        std::vector<Dx12Texture> m_BackBuffers;
        Views::RenderTarget      m_RenderTargets;

        EResourceFormat m_BackbufferFormat = EResourceFormat::Unknown;

        //

        Ptr<IRootSignature> m_RootSignature;
        Ptr<IPipelineState> m_PipelineState;
    };
} // namespace Neon::RHI