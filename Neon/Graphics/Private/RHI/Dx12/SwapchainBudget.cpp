#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>
#include <Private/RHI/Dx12/Commands/Commands.hpp>

namespace Neon::RHI
{
    std::vector<ICommandList*> Dx12Swapchain::AllocateCommandLists(
        D3D12_COMMAND_LIST_TYPE Type,
        size_t                  Count)
    {
        return m_BudgetManager.AllocateCommandLists(Type, Count);
    }

    void Dx12Swapchain::FreeCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        m_BudgetManager.FreeCommandLists(Type, Commands);
    }

    void Dx12Swapchain::ResetCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        m_BudgetManager.ResetCommandLists(Type, Commands);
    }

    GraphicsMemoryAllocator* Dx12Swapchain::GetAllocator()
    {
        return &m_MemoryAllocator;
    }

    ICommandQueue* Dx12Swapchain::GetQueue(CommandQueueType Type)
    {
        return &m_BudgetManager.GetQueueManager()->Get(CastCommandQueueType(Type))->Queue;
    }

    IResourceStateManager* Dx12Swapchain::GetStateManager()
    {
        return m_MemoryAllocator.GetStateManager();
    }
} // namespace Neon::RHI
