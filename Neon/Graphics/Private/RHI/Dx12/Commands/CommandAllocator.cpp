#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/CommandAllocator.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Private/RHI/Dx12/Commands/Commands.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    ICommandAllocator* ICommandAllocator::Create(
        CommandQueueType Type)
    {
        return NEON_NEW Dx12CommandAllocator(Type);
    }

    Dx12CommandAllocator::Dx12CommandAllocator(
        CommandQueueType Type)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateCommandAllocator(
            CastCommandQueueType(Type),
            IID_PPV_ARGS(&m_CommandAllocator)));
    }

    void Dx12CommandAllocator::Reset()
    {
        ThrowIfFailed(m_CommandAllocator->Reset());
    }

    ID3D12CommandAllocator* Dx12CommandAllocator::Get()
    {
        return m_CommandAllocator.Get();
    }
} // namespace Neon::RHI