#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>
#include <Private/RHI/Dx12/Commands/Commands.hpp>

namespace Neon::RHI
{
    std::vector<ICommandList*> Dx12Swapchain::AllocateCommandLists(
        D3D12_COMMAND_LIST_TYPE Type,
        size_t                  Count)
    {
        return m_FrameManager->AllocateCommandLists(Type, Count);
    }

    void Dx12Swapchain::FreeCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        m_FrameManager->FreeCommandLists(Type, Commands);
    }

    void Dx12Swapchain::ResetCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        m_FrameManager->ResetCommandLists(Type, Commands);
    }

    ICommandQueue* Dx12Swapchain::GetQueue(
        CommandQueueType Type)
    {
        return &m_FrameManager->GetQueueManager()->Get(CastCommandQueueType(Type))->Queue;
    }

    Dx12Swapchain* Dx12Swapchain::Get()
    {
        return static_cast<Dx12Swapchain*>(IRenderDevice::Get()->GetSwapchain());
    }

    void Dx12Swapchain::SafeRelease(
        const Dx12Buffer::Handle& Handle)
    {
        m_FrameManager->SafeRelease(Handle);
    }

    void Dx12Swapchain::SafeRelease(
        const Win32::ComPtr<ID3D12DescriptorHeap>& Resource)
    {
        m_FrameManager->SafeRelease(Resource);
    }

    void Dx12Swapchain::SafeRelease(
        const Win32::ComPtr<ID3D12Resource>&      Resource,
        const Win32::ComPtr<D3D12MA::Allocation>& Allocation)
    {
        m_FrameManager->SafeRelease(Resource, Allocation);
    }

    void Dx12Swapchain::WaitForCopy(
        ICommandQueue* Queue,
        uint64_t       FenceValue)
    {
        m_FrameManager->WaitForCopy(static_cast<Dx12CommandQueue*>(Queue), FenceValue);
    }

    uint64_t Dx12Swapchain::EnqueueRequestCopy(
        std::function<void(ICopyCommandList*)> Task)
    {
        return m_FrameManager->RequestCopy(std::move(Task));
    }

    Dx12FrameDescriptorHeap* Dx12Swapchain::GetFrameDescriptorAllocator(
        DescriptorType Type) noexcept
    {
        return m_FrameManager->GetFrameDescriptorAllocator(Type);
    }

    Dx12StagedDescriptorHeap* Dx12Swapchain::GetStagedDescriptorAllocator(
        DescriptorType Type) noexcept
    {
        return m_FrameManager->GetStagedDescriptorAllocator(Type);
    }

    Dx12StaticDescriptorHeap* Dx12Swapchain::GetStaticDescriptorAllocator(
        DescriptorType Type) noexcept
    {
        return &m_StaticDescriptors[size_t(Type)];
    }
} // namespace Neon::RHI
