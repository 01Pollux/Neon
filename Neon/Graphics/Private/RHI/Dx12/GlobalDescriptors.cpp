#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/GlobalDescriptors.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    Dx12DFrameDescriptorHeapBuddyAllocator::BuddyBlock::BuddyBlock(
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
        uint32_t                   SizeOfHeap) :
        Heap(DescriptorType, SizeOfHeap, true),
        Allocator(SizeOfHeap)
    {
    }

    Dx12DFrameDescriptorHeapBuddyAllocator::Dx12DFrameDescriptorHeapBuddyAllocator(
        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType,
        uint32_t                   SizeOfHeap) :
        m_HeapBlock(DescriptorType, SizeOfHeap)
    {
    }

    Dx12DFrameDescriptorHeapBuddyAllocator::~Dx12DFrameDescriptorHeapBuddyAllocator()
    {
        m_HeapBlock.Heap.SilentDelete();
    }

    DescriptorHeapHandle Dx12DFrameDescriptorHeapBuddyAllocator::Allocate(
        uint32_t DescriptorSize)
    {
        std::scoped_lock HeapLock(m_HeapBlockMutex);

        if (auto Hndl = m_HeapBlock.Allocator.Allocate(DescriptorSize)) [[likely]]
        {
            return {
                .Heap   = &m_HeapBlock.Heap,
                .Offset = uint32_t(Hndl.Offset),
                .Size   = uint32_t(Hndl.Size)
            };
        }

        const char* Type = "???";
        switch (m_HeapBlock.Heap.GetType())
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            Type = "CBV_SRV_UAV";
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            Type = "Sampler";
            break;
        }
        NEON_FATAL("Descriptor Heap is full");
        NEON_FATAL("Try to increase the size of the '{}' heap", Type);
        std::unreachable();
    }

    void Dx12DFrameDescriptorHeapBuddyAllocator::FreeAll()
    {
        m_HeapBlock.Allocator = Allocator::BuddyAllocator(m_HeapBlock.Heap.GetSize());
    }

    Dx12DescriptorHeap* Dx12DFrameDescriptorHeapBuddyAllocator::GetHeap()
    {
        return &m_HeapBlock.Heap;
    }

    //

    IStaticDescriptorHeap* IStaticDescriptorHeap::Get(
        DescriptorType Type)
    {
        return Dx12Swapchain::Get()->GetStaticDescriptorAllocator(Type);
    }

    Dx12StaticDescriptorHeap::Dx12StaticDescriptorHeap(
        DescriptorType Type) :
        m_Allocator(
            CastDescriptorType(Type),
            Type == DescriptorType::ResourceView       ? IRenderDevice::GetDescriptorSize().Static_Resource
            : Type == DescriptorType::RenderTargetView ? IRenderDevice::GetDescriptorSize().Static_Rtv
            : Type == DescriptorType::RenderTargetView ? IRenderDevice::GetDescriptorSize().Static_Dsv
                                                       : IRenderDevice::GetDescriptorSize().Static_Sampler,
            false)
    {
    }

    Dx12StaticDescriptorHeap::~Dx12StaticDescriptorHeap()
    {
        for (auto Heap : m_Allocator.GetAllHeaps())
        {
            Heap->SilentDelete();
        }
    }

    DescriptorHeapHandle Dx12StaticDescriptorHeap::Allocate(
        uint32_t Count)
    {
        return m_Allocator.Allocate(Count);
    }

    void Dx12StaticDescriptorHeap::Free(
        std::span<const DescriptorHeapHandle> Handles)
    {
        m_Allocator.Free(Handles);
    }

    //

    IStagedDescriptorHeap* IStagedDescriptorHeap::Get(
        DescriptorType Type)
    {
        return Dx12Swapchain::Get()->GetStagedDescriptorAllocator(Type);
    }

    Dx12StagedDescriptorHeap::Dx12StagedDescriptorHeap(
        DescriptorType Type) :
        m_Allocator(
            CastDescriptorType(Type),
            Type == DescriptorType::ResourceView       ? IRenderDevice::GetDescriptorSize().Staged_Resource
            : Type == DescriptorType::RenderTargetView ? IRenderDevice::GetDescriptorSize().Staged_Rtv
            : Type == DescriptorType::RenderTargetView ? IRenderDevice::GetDescriptorSize().Staged_Dsv
                                                       : IRenderDevice::GetDescriptorSize().Staged_Sampler,
            false)
    {
    }

    Dx12StagedDescriptorHeap::~Dx12StagedDescriptorHeap()
    {
        for (auto Heap : m_Allocator.GetAllHeaps())
        {
            Heap->SilentDelete();
        }
    }

    DescriptorHeapHandle Dx12StagedDescriptorHeap::Allocate(
        uint32_t Count)
    {
        return m_Allocator.Allocate(Count);
    }

    void Dx12StagedDescriptorHeap::Free(
        std::span<const DescriptorHeapHandle> Handles)
    {
        m_Allocator.Free(Handles);
    }

    void Dx12StagedDescriptorHeap::Reset()
    {
        m_Allocator.FreeAll();
    }

    //

    IFrameDescriptorHeap* IFrameDescriptorHeap::Get(
        DescriptorType Type)
    {
        return Dx12Swapchain::Get()->GetFrameDescriptorAllocator(Type);
    }

    Dx12FrameDescriptorHeap::Dx12FrameDescriptorHeap(
        DescriptorType Type) :
        m_Allocator(
            CastDescriptorType(Type),
            Type == DescriptorType::ResourceView ? IRenderDevice::GetDescriptorSize().Frame_Resource
                                                 : IRenderDevice::GetDescriptorSize().Frame_Sampler)
    {
    }

    DescriptorHeapHandle Dx12FrameDescriptorHeap::Allocate(
        uint32_t Count)
    {
        return m_Allocator.Allocate(Count);
    }

    Dx12DescriptorHeap* Dx12FrameDescriptorHeap::GetHeap()
    {
        return m_Allocator.GetHeap();
    }

    void Dx12FrameDescriptorHeap::Reset()
    {
        m_Allocator.FreeAll();
    }
} // namespace Neon::RHI