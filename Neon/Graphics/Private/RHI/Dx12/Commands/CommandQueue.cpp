#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Private/RHI/Dx12/Commands/Commands.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Commands/CommandAllocator.hpp>

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

    ICommandList* Dx12CommandQueue::AllocateCommandList(
        CommandQueueType Type)
    {
        ICommandList* CommandList = nullptr;
        auto Allocator            = NEON_NEW Dx12CommandAllocator(Type);

        switch (Type)
        {
        case CommandQueueType::Graphics:
            CommandList = NEON_NEW Dx12GraphicsCommandList(Allocator, Type);
            break;
        default:
            break;
        }

        if (CommandList)
        {
            Allocator->Reset();
            CommandList->Reset(Allocator);
        }
        return CommandList;
    }

    std::vector<ICommandList*> Dx12CommandQueue::AllocateCommandLists(
        CommandQueueType Type,
        size_t           Count)
    {
        return std::vector<ICommandList*>();
    }

    void Dx12CommandQueue::Upload(
        ICommandList* Command)
    {
        Command->Close();
        auto Dx12Command = static_cast<ID3D12CommandList*>(dynamic_cast<Dx12CommandList*>(Command)->Get());
        m_CommandQueue->ExecuteCommandLists(1, &Dx12Command);
    }

    void Dx12CommandQueue::Upload(
        const std::vector<ICommandList*>& Commands)
    {
    }

    ID3D12CommandQueue* Dx12CommandQueue::Get()
    {
        return m_CommandQueue.Get();
    }
} // namespace Neon::RHI