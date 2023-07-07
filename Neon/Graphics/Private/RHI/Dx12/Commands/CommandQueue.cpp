#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Private/RHI/Dx12/Commands/Commands.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>

#include <Private/RHI/Dx12/Swapchain.hpp>

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

    std::vector<ICommandList*> Dx12CommandQueue::AllocateCommandLists(
        CommandQueueType Type,
        size_t           Count)
    {
        return Dx12Swapchain::Get()->AllocateCommandLists(CastCommandQueueType(Type), Count);
    }

    void Dx12CommandQueue::FreeCommandLists(
        CommandQueueType         Type,
        std::span<ICommandList*> Commands)
    {
        Dx12Swapchain::Get()->FreeCommandLists(CastCommandQueueType(Type), Commands);
    }

    void Dx12CommandQueue::Upload(
        std::span<ICommandList*> Commands)
    {
        std::vector<ID3D12CommandList*> Dx12Commands;
        Dx12Commands.reserve(Commands.size());
        std::ranges::transform(
            Commands, std::back_inserter(Dx12Commands),
            [](ICommandList* Command)
            {
                auto Dx12Command = static_cast<ID3D12GraphicsCommandList*>(dynamic_cast<Dx12CommandList*>(Command)->Get());
                Dx12Command->Close();
                return static_cast<ID3D12CommandList*>(Dx12Command);
            });

        m_CommandQueue->ExecuteCommandLists(UINT(Dx12Commands.size()), Dx12Commands.data());
    }

    void Dx12CommandQueue::Reset(
        CommandQueueType         Type,
        std::span<ICommandList*> Commands)
    {
        Dx12Swapchain::Get()->ResetCommandLists(CastCommandQueueType(Type), Commands);
    }

    ID3D12CommandQueue* Dx12CommandQueue::Get()
    {
        return m_CommandQueue.Get();
    }
} // namespace Neon::RHI