#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Commands/CommandAllocator.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Private/RHI/Dx12/Commands/Commands.hpp>

namespace Neon::RHI
{
    ICommandList* ICommandList::Create(
        ICommandAllocator* Allocator,
        CommandQueueType   Type)
    {
        return NEON_NEW Dx12CommandList(Allocator, Type);
    }

    Dx12CommandList::Dx12CommandList(
        ICommandAllocator* Allocator,
        CommandQueueType   Type)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateCommandList(
            0,
            CastCommandQueueType(Type),
            static_cast<Dx12CommandAllocator*>(Allocator)->Get(),
            nullptr,
            IID_PPV_ARGS(&m_CommandList)));
        ThrowIfFailed(m_CommandList->Close());
    }

    void Dx12CommandList::Reset(
        ICommandAllocator* Allocator)
    {
        m_CommandList->Reset(static_cast<Dx12CommandAllocator*>(Allocator)->Get(), nullptr);
    }

    ID3D12GraphicsCommandList* Dx12CommandList::Get()
    {
        return m_CommandList.Get();
    }
} // namespace Neon::RHI