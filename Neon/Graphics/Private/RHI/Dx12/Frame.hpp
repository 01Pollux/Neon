#pragma once

#include <Private/RHI/Dx12/Resource/Descriptor.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Allocator/FreeList.hpp>

namespace Neon::RHI
{
    class Dx12Swapchain;

    class FrameResource
    {
    public:
        static constexpr size_t NumCommandContextTypes = 3;

    private:
        struct CommandAllocatorInstance
        {
            Win32::ComPtr<ID3D12CommandAllocator> CommandAllocator;

            CommandAllocatorInstance(
                D3D12_COMMAND_LIST_TYPE CommandType);
        };
        using Dx12CommandAllocatorPool  = Allocator::FreeList<CommandAllocatorInstance>;
        using Dx12CommandAllocatorPools = std::array<Dx12CommandAllocatorPool, NumCommandContextTypes>;

    public:
        /// <summary>
        /// Request command allocator from pool
        /// </summary>
        ID3D12CommandAllocator* RequestAllocator(
            D3D12_COMMAND_LIST_TYPE CommandType);

        /// <summary>
        /// Release all stale resources
        /// </summary>
        void Reset(
            Dx12Swapchain* Swapchain);

        /// <summary>
        /// Enqueue descriptor heap to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Ptr<IDescriptorHeap>& Heap);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Ptr<IDescriptorHeapAllocator>& Allocator,
            const DescriptorHeapHandle&          Handle);

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
        /// Convert index to command list type
        /// </summary>
        static D3D12_COMMAND_LIST_TYPE GetCommandListType(
            size_t Index) noexcept;

        /// <summary>
        /// Convert command list type to index
        /// </summary>
        static size_t GetCommandListIndex(
            D3D12_COMMAND_LIST_TYPE Type) noexcept;

    private:
        Dx12CommandAllocatorPools m_AllocatorsPools;

        std::map<Ptr<IDescriptorHeapAllocator>, std::vector<DescriptorHeapHandle>> m_DescriptorHeapHandles;

        std::vector<Ptr<IDescriptorHeap>>          m_DescriptorHeaps;
        std::vector<Dx12Buffer::Handle>            m_Buffers;
        std::vector<Win32::ComPtr<ID3D12Resource>> m_Resources;
    };
} // namespace Neon::RHI