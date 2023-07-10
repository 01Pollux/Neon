#pragma once

#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <Private/RHI/Dx12/GlobalDescriptors.hpp>
#include <Allocator/FreeList.hpp>

namespace Neon::RHI
{
    class FrameResource
    {
    public:
        struct CommandAllocatorInstance
        {
            WinAPI::ComPtr<ID3D12CommandAllocator> CommandAllocator;

            CommandAllocatorInstance(
                D3D12_COMMAND_LIST_TYPE CommandType);
        };
        using Dx12CommandAllocatorPool  = Allocator::FreeList<CommandAllocatorInstance>;
        using Dx12CommandAllocatorPools = std::array<Dx12CommandAllocatorPool, 2>;

    public:
        /// <summary>
        /// Request command allocator from pool
        /// </summary>
        ID3D12CommandAllocator* RequestAllocator(
            D3D12_COMMAND_LIST_TYPE CommandType);

        /// <summary>
        /// Release all stale resources
        /// </summary>
        void Reset();

        /// <summary>
        /// Enqueue a descriptor handle to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            IDescriptorHeapAllocator*   Allocator,
            const DescriptorHeapHandle& Handle);

        /// <summary>
        /// Enqueue buffer to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Dx12Buffer::Handle& Handle);

        /// <summary>
        /// Enqueue descriptor to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const WinAPI::ComPtr<ID3D12DescriptorHeap>& Descriptor);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const WinAPI::ComPtr<ID3D12Resource>&      Resource,
            const WinAPI::ComPtr<D3D12MA::Allocation>& Allocation);

    public:
        /// <summary>
        /// Get frame descriptor heap allocator
        /// </summary>
        [[nodiscard]] Dx12FrameDescriptorHeap* GetFrameDescriptorAllocator(
            DescriptorType Type) noexcept;

        /// <summary>
        /// Get staged descriptor heap allocator
        /// </summary>
        [[nodiscard]] Dx12StagedDescriptorHeap* GetStagedDescriptorAllocator(
            DescriptorType Type) noexcept;

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
        Dx12FrameDescriptorHeap m_FrameDescriptors[2]{
            DescriptorType::ResourceView,
            DescriptorType::Sampler
        };
        Dx12StagedDescriptorHeap m_StagedDescriptors[size_t(DescriptorType::Count)]{
            DescriptorType::ResourceView,
            DescriptorType::RenderTargetView,
            DescriptorType::DepthStencilView,
            DescriptorType::Sampler
        };

        Dx12CommandAllocatorPools m_AllocatorsPools;

        std::map<IDescriptorHeapAllocator*, std::vector<DescriptorHeapHandle>> m_DescriptorHeapHandles;

        std::vector<Dx12Buffer::Handle>                  m_Buffers;
        std::vector<WinAPI::ComPtr<ID3D12Object>>        m_Resources;
        std::vector<WinAPI::ComPtr<D3D12MA::Allocation>> m_Allocation;
    };
} // namespace Neon::RHI