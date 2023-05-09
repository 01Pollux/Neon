#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/CommandsQueue.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Private/RHI/Dx12/Commands/Commands.hpp>

namespace Neon::RHI
{
    ICommandQueue* ICommandQueue::Create(
        CommandQueueType Type)
    {
        return NEON_NEW Dx12CommandQueue(Type);
    }

    Dx12CommandQueue::Dx12CommandQueue(
        CommandQueueType Type)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();

        D3D12_COMMAND_QUEUE_DESC Desc{
            .Type = CastCommandQueueType(Type),
        };
        ThrowIfFailed(Dx12Device->CreateCommandQueue(
            &Desc,
            IID_PPV_ARGS(&m_CommandQueue)));
    }

    ID3D12CommandQueue* Dx12CommandQueue::Get()
    {
        return m_CommandQueue.Get();
    }
} // namespace Neon::RHI