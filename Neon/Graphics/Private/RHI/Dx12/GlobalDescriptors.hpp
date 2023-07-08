#pragma once

#include <RHI/GlobalDescriptors.hpp>
#include <Private/RHI/Dx12/Resource/Descriptor.hpp>

namespace Neon::RHI
{
    class Dx12DFrameDescriptorHeapBuddyAllocator
    {
        struct BuddyBlock
        {
            Dx12DescriptorHeap        Heap;
            Allocator::BuddyAllocator Allocator;

            BuddyBlock(
                D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
                uint32_t                   SizeOfHeap);
        };

    public:
        Dx12DFrameDescriptorHeapBuddyAllocator(
            D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
            uint32_t                   SizeOfHeap);

        NEON_CLASS_NO_COPYMOVE(Dx12DFrameDescriptorHeapBuddyAllocator);

        ~Dx12DFrameDescriptorHeapBuddyAllocator();

        DescriptorHeapHandle Allocate(
            uint32_t DescriptorSize);

        void FreeAll();

        /// <summary>
        /// Returns the underlying heap.
        /// </summary>
        [[nodiscard]] ID3D12DescriptorHeap* GetHeap();

    private:
        std::mutex m_HeapBlockMutex;
        BuddyBlock m_HeapBlock;
    };

    //

    class Dx12StaticDescriptorHeap : public IStaticDescriptorHeap
    {
    public:
        Dx12StaticDescriptorHeap(
            DescriptorType Type);
        NEON_CLASS_NO_COPYMOVE(Dx12StaticDescriptorHeap);
        ~Dx12StaticDescriptorHeap() override;

        DescriptorHeapHandle Allocate(
            uint32_t Count) override;

        void Free(
            std::span<const DescriptorHeapHandle> Handles) override;

    private:
        Dx12DescriptorHeapBuddyAllocator m_Allocator;
    };

    //

    class Dx12StagedDescriptorHeap : public IStagedDescriptorHeap
    {
    public:
        Dx12StagedDescriptorHeap(
            DescriptorType Type);
        NEON_CLASS_NO_COPYMOVE(Dx12StagedDescriptorHeap);
        ~Dx12StagedDescriptorHeap() override;

        DescriptorHeapHandle Allocate(
            uint32_t Count) override;

        void Free(
            std::span<const DescriptorHeapHandle> Handles) override;

        /// <summary>
        /// Resets the allocator.
        /// </summary>
        void Reset();

    private:
        Dx12DescriptorHeapBuddyAllocator m_Allocator;
    };

    //

    class Dx12FrameDescriptorHeap : public IFrameDescriptorHeap
    {
    public:
        Dx12FrameDescriptorHeap(
            DescriptorType Type);
        NEON_CLASS_NO_COPYMOVE(Dx12FrameDescriptorHeap);
        ~Dx12FrameDescriptorHeap() = default;

        DescriptorHeapHandle Allocate(
            uint32_t Count) override;

        /// <summary>
        /// Returns the underlying heap.
        /// </summary>
        [[nodiscard]] ID3D12DescriptorHeap* GetHeap();

        /// <summary>
        /// Resets the allocator.
        /// </summary>
        void Reset();

    private:
        Dx12DFrameDescriptorHeapBuddyAllocator m_Allocator;
    };
} // namespace Neon::RHI