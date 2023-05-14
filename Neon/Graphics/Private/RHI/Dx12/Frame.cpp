#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Frame.hpp>
#include <Private/RHI/Dx12/Device.hpp>

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