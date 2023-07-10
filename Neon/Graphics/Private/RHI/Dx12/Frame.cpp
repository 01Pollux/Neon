#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Frame.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

namespace Neon::RHI
{
    FrameResource::CommandAllocatorInstance::CommandAllocatorInstance(
        D3D12_COMMAND_LIST_TYPE CommandType)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateCommandAllocator(
            CommandType,
            IID_PPV_ARGS(&CommandAllocator)));
    }

    ID3D12CommandAllocator* FrameResource::RequestAllocator(
        D3D12_COMMAND_LIST_TYPE CommandType)
    {
        auto Allocator = m_AllocatorsPools[GetCommandListIndex(CommandType)].Allocate(CommandType)->CommandAllocator.Get();
        ThrowIfFailed(Allocator->Reset());
        return Allocator;
    }

    void FrameResource::Reset()
    {
        for (auto& [HeapAllocators, Handles] : m_DescriptorHeapHandles)
        {
            HeapAllocators->Free(Handles);
        }

        auto Allocator = Dx12RenderDevice::Get()->GetAllocator();
        Allocator->FreeBuffers(m_Buffers);

        m_DescriptorHeapHandles.clear();
        m_Buffers.clear();
        m_Resources.clear();
        m_Allocation.clear();

        for (auto& Pool : m_AllocatorsPools)
        {
            Pool.ClearActives();
        }

        for (auto& Pool : m_FrameDescriptors)
        {
            Pool.Reset();
        }

        for (auto& Pool : m_StagedDescriptors)
        {
            Pool.Reset();
        }
    }

    void FrameResource::SafeRelease(
        IDescriptorHeapAllocator*   Allocator,
        const DescriptorHeapHandle& Handle)
    {
        m_DescriptorHeapHandles[Allocator].emplace_back(Handle);
    }

    void FrameResource::SafeRelease(
        const Dx12Buffer::Handle& Handle)
    {
        m_Buffers.emplace_back(Handle);
    }

    void FrameResource::SafeRelease(
        const WinAPI::ComPtr<ID3D12DescriptorHeap>& Descriptor)
    {
        // First we check if the incoming descriptor exists in the to be deleted handles.
        // If it does, no need to remove the handle, just remove the descriptor instead.
        for (auto Iter = m_DescriptorHeapHandles.begin(); Iter != m_DescriptorHeapHandles.end(); Iter++)
        {
            auto Contains = std::ranges::find_if(
                                Iter->second,
                                [Descriptor](const DescriptorHeapHandle& Handle)
                                { return static_cast<Dx12DescriptorHeap*>(Handle.Heap)->Get() == Descriptor.Get(); }) != Iter->second.end();

            if (Contains)
            {
                m_DescriptorHeapHandles.erase(Iter);
                break;
            }
        }
        m_Resources.emplace_back(Descriptor);
    }

    void FrameResource::SafeRelease(
        const WinAPI::ComPtr<ID3D12Resource>&      Resource,
        const WinAPI::ComPtr<D3D12MA::Allocation>& Allocation)
    {
        m_Resources.emplace_back(Resource);
        if (Allocation)
            m_Allocation.emplace_back(Allocation);
    }

    Dx12FrameDescriptorHeap* FrameResource::GetFrameDescriptorAllocator(
        DescriptorType Type) noexcept
    {
        return &m_FrameDescriptors[Type == DescriptorType::ResourceView ? 0 : 1];
    }

    Dx12StagedDescriptorHeap* FrameResource::GetStagedDescriptorAllocator(
        DescriptorType Type) noexcept
    {
        return &m_StagedDescriptors[size_t(Type)];
    }

    //

    D3D12_COMMAND_LIST_TYPE FrameResource::GetCommandListType(
        size_t Index) noexcept
    {
        switch (Index)
        {
        case 0:
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case 1:
            return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        default:
            std::unreachable();
        }
    }

    size_t FrameResource::GetCommandListIndex(
        D3D12_COMMAND_LIST_TYPE Type) noexcept
    {
        switch (Type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            return 0;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            return 1;
        default:
            std::unreachable();
        }
    }
} // namespace Neon::RHI