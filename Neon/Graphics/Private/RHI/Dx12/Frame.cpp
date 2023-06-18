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

    void FrameResource::Reset(
        Dx12Swapchain* Swapchain)
    {
        for (auto& [HeapAllocators, Handles] : m_DescriptorHeapHandles)
        {
            HeapAllocators->Free(Handles);
        }

        auto Allocator = Swapchain->GetAllocator();
        Allocator->FreeBuffers(m_Buffers);

        m_DescriptorHeapHandles.clear();
        m_Buffers.clear();
        m_Resources.clear();
        m_Allocation.clear();

        for (auto& Allocator : m_AllocatorsPools)
        {
            Allocator.ClearActives();
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
        const Win32::ComPtr<ID3D12DescriptorHeap>& Descriptor)
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
        const Win32::ComPtr<ID3D12Resource>&      Resource,
        const Win32::ComPtr<D3D12MA::Allocation>& Allocation)
    {
        m_Resources.emplace_back(Resource);
        if (Allocation)
            m_Allocation.emplace_back(Allocation);
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