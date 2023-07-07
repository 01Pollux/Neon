#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>
#include <Private/RHI/Dx12/Commands/Commands.hpp>

namespace Neon::RHI
{
    std::vector<ICommandList*> Dx12Swapchain::AllocateCommandLists(
        D3D12_COMMAND_LIST_TYPE Type,
        size_t                  Count)
    {
        return m_BudgetManager->AllocateCommandLists(Type, Count);
    }

    void Dx12Swapchain::FreeCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        m_BudgetManager->FreeCommandLists(Type, Commands);
    }

    void Dx12Swapchain::ResetCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        m_BudgetManager->ResetCommandLists(Type, Commands);
    }

    ICommandQueue* Dx12Swapchain::GetQueue(
        CommandQueueType Type)
    {
        return &m_BudgetManager->GetQueueManager()->Get(CastCommandQueueType(Type))->Queue;
    }

    IDescriptorHeapAllocator* Dx12Swapchain::GetDescriptorHeapManager(
        DescriptorType Type,
        bool           Dynamic)
    {
        auto Dx12Type = CastDescriptorType(Type);
        return Dynamic ? &(*m_DynamicDescriptorHeap)[Dx12Type]
                       : &(*m_StaticDescriptorHeap)[Dx12Type];
    }

    Dx12Swapchain* Dx12Swapchain::Get()
    {
        return static_cast<Dx12Swapchain*>(IRenderDevice::Get()->GetSwapchain());
    }

    void Dx12Swapchain::SafeRelease(
        IDescriptorHeapAllocator*   Allocator,
        const DescriptorHeapHandle& Handle)
    {
        m_BudgetManager->SafeRelease(Allocator, Handle);
    }

    void Dx12Swapchain::SafeRelease(
        const Dx12Buffer::Handle& Handle)
    {
        m_BudgetManager->SafeRelease(Handle);
    }

    void Dx12Swapchain::SafeRelease(
        const Win32::ComPtr<ID3D12DescriptorHeap>& Resource)
    {
        m_BudgetManager->SafeRelease(Resource);
    }

    void Dx12Swapchain::SafeRelease(
        const Win32::ComPtr<ID3D12Resource>&      Resource,
        const Win32::ComPtr<D3D12MA::Allocation>& Allocation)
    {
        m_BudgetManager->SafeRelease(Resource, Allocation);
    }

    void Dx12Swapchain::WaitForCopy(
        ICommandQueue* Queue,
        uint64_t       FenceValue)
    {
        m_BudgetManager->WaitForCopy(static_cast<Dx12CommandQueue*>(Queue), FenceValue);
    }

    uint64_t Dx12Swapchain::EnqueueRequestCopy(
        std::function<void(ICopyCommandList*)> Task)
    {
        return m_BudgetManager->RequestCopy(std::move(Task));
    }
} // namespace Neon::RHI
