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

    ICommandQueue* Dx12Swapchain::GetQueue(
        CommandQueueType Type)
    {
        return &m_BudgetManager.GetQueueManager()->Get(CastCommandQueueType(Type))->Queue;
    }

    IResourceStateManager* Dx12Swapchain::GetStateManager()
    {
        return m_MemoryAllocator.GetStateManager();
    }

    IDescriptorHeapAllocator* Dx12Swapchain::GetDescriptorHeapManager(
        DescriptorType Type,
        bool           Dynamic)
    {
        return m_BudgetManager.GetDescriptorHeapManager(CastDescriptorType(Type), Dynamic);
    }

    void Dx12Swapchain::SafeRelease(
        IDescriptorHeapAllocator*   Allocator,
        const DescriptorHeapHandle& Handle)
    {
        m_BudgetManager.SafeRelease(Allocator, Handle);
    }

    void Dx12Swapchain::SafeRelease(
        const Dx12Buffer::Handle& Handle)
    {
        m_BudgetManager.SafeRelease(Handle);
    }

    void Dx12Swapchain::SafeRelease(
        const Win32::ComPtr<ID3D12DescriptorHeap>& Resource)
    {
        m_BudgetManager.SafeRelease(Resource);
    }

    void Dx12Swapchain::SafeRelease(
        const Win32::ComPtr<ID3D12Resource>& Resource)
    {
        m_BudgetManager.SafeRelease(Resource);
    }
} // namespace Neon::RHI
