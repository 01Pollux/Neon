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
        auto Allocator = m_AllocatorsPools[CommandType].Allocate(CommandType)->CommandAllocator.Get();
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

        m_DescriptorHeaps.clear();
        m_DescriptorHeapHandles.clear();
        m_Buffers.clear();
        m_Resources.clear();
    }

    void FrameResource::SafeRelease(
        const Ptr<IDescriptorHeap>& Heap)
    {
        m_DescriptorHeaps.emplace_back(Heap);
    }

    void FrameResource::SafeRelease(
        const Ptr<IDescriptorHeapAllocator>& Allocator,
        const DescriptorHeapHandle&          Handle)
    {
        m_DescriptorHeapHandles[Allocator].emplace_back(Handle);
    }

    void FrameResource::SafeRelease(
        const Dx12Buffer::Handle& Handle)
    {
        m_Buffers.emplace_back(Handle);
    }

    void FrameResource::SafeRelease(
        const Win32::ComPtr<ID3D12Resource>& Resource)
    {
        m_Resources.emplace_back(Resource);
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
        case 2:
            return D3D12_COMMAND_LIST_TYPE_COPY;
        }
        return D3D12_COMMAND_LIST_TYPE(-1);
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
        case D3D12_COMMAND_LIST_TYPE_COPY:
            return 2;
        }
        return size_t(-1);
    }
} // namespace Neon::RHI