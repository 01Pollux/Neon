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
        bool IsDirect)
    {
        return m_FrameManager->GetQueue(IsDirect);
    }

    IFence* Dx12Swapchain::GetQueueFence(
        bool IsDirect)
    {
        return m_FrameManager->GetQueueFence(IsDirect);
    }

    Dx12Swapchain* Dx12Swapchain::Get()
    {
        return static_cast<Dx12Swapchain*>(IRenderDevice::Get()->GetSwapchain());
    }

    void Dx12Swapchain::SafeRelease(
        const GraphicsMemoryAllocator::Handle& Handle)
    {
        m_FrameManager->SafeRelease(Handle);
    }

    void Dx12Swapchain::SafeRelease(
        const WinAPI::ComPtr<ID3D12DescriptorHeap>& Resource)
    {
        m_FrameManager->SafeRelease(Resource);
    }

    void Dx12Swapchain::SafeRelease(
        const WinAPI::ComPtr<ID3D12Resource>&      Resource,
        const WinAPI::ComPtr<D3D12MA::Allocation>& Allocation)
    {
        m_FrameManager->SafeRelease(Resource, Allocation);
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
