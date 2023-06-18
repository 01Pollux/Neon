#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Budget.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
    static constexpr uint32_t SizeOfCPUDescriptor_Resource = 256;
    static constexpr uint32_t SizeOfCPUDescriptor_Samplers = 8;
    static constexpr uint32_t SizeOfCPUDescriptor_Rtv      = 64;
    static constexpr uint32_t SizeOfCPUDescriptor_Dsv      = 16;

    static constexpr uint32_t SizeOfGPUDescriptor_Resource = 16'384;
    static constexpr uint32_t SizeOfGPUDescriptor_Samplers = 32;
    static constexpr uint32_t SizeOfGPUDescriptor_Rtv      = 24;
    static constexpr uint32_t SizeOfGPUDescriptor_Dsv      = 16;

    //

    Dx12CommandQueueManager::QueueAndFence::QueueAndFence(
        ISwapchain*      Swapchain,
        CommandQueueType QueueType) :
        Queue(Swapchain, QueueType),
        Fence(0)
    {
    }

    //

    Dx12CommandQueueManager::Dx12CommandQueueManager(
        ISwapchain* Swapchain) :
        m_Graphics(Swapchain, CommandQueueType::Graphics),
        m_Compute(Swapchain, CommandQueueType::Compute)
    {
    }

    auto Dx12CommandQueueManager::GetGraphics() -> QueueAndFence*
    {
        return &m_Graphics;
    }

    auto Dx12CommandQueueManager::GetCompute() -> QueueAndFence*
    {
        return &m_Compute;
    }

    auto Dx12CommandQueueManager::Get(
        D3D12_COMMAND_LIST_TYPE CommandType) -> QueueAndFence*
    {
        switch (CommandType)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            return &m_Graphics;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            return &m_Compute;
        default:
            std::unreachable();
        }
    }

    //

    Dx12CommandContextManager::CommandListInstance::CommandListInstance(
        ISwapchain*             Swapchain,
        ID3D12CommandAllocator* Allocator,
        D3D12_COMMAND_LIST_TYPE CommandType)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateCommandList(
            0,
            CommandType,
            Allocator,
            nullptr,
            IID_PPV_ARGS(&Dx12CmdList)));
        Dx12CmdList->Close();

        switch (CommandType)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            CommandList.reset(NEON_NEW Dx12GraphicsCommandList(Swapchain));
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            CommandList.reset(NEON_NEW Dx12ComputeCommandList(Swapchain));
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            CommandList.reset(NEON_NEW Dx12CopyCommandList(Swapchain));
            break;
        default:
            std::unreachable();
        }
    }

    Dx12CommandContextManager::Dx12CommandContextManager(
        D3D12_COMMAND_LIST_TYPE Type) :
        m_Type(Type)
    {
    }

    ICommandList* Dx12CommandContextManager::Request(
        ISwapchain*             Swapchain,
        ID3D12CommandAllocator* Allocator)
    {
        auto Iter = m_CommandListsPool.Allocate(Swapchain, Allocator, m_Type);
        ThrowIfFailed(Iter->Dx12CmdList->Reset(Allocator, nullptr));

        Iter->CommandList->AttachCommandList(Iter->Dx12CmdList.Get());
        m_ToPoolMap.emplace(Iter->CommandList.get(), Iter);

        return Iter->CommandList.get();
    }

    ICommandList* Dx12CommandContextManager::Request(
        ISwapchain*    Swapchain,
        FrameResource& Frame)
    {
        return Request(Swapchain, Frame.RequestAllocator(m_Type));
    }

    void Dx12CommandContextManager::Free(
        ICommandList* CommandList)
    {
        auto Iter = m_ToPoolMap.find(CommandList);
        m_CommandListsPool.Free(Iter->second);
        m_ToPoolMap.erase(Iter);
    }

    void Dx12CommandContextManager::Reset(
        ID3D12CommandAllocator* Allocator,
        ICommandList*           CommandList)
    {
        ThrowIfFailed(m_ToPoolMap[CommandList]->Dx12CmdList->Reset(Allocator, nullptr));
    }

    void Dx12CommandContextManager::Reset(
        FrameResource& Frame,
        ICommandList*  CommandList)
    {
        return Reset(Frame.RequestAllocator(m_Type), CommandList);
    }

    //

    BudgetManager::BudgetManager(
        ISwapchain* Swapchain) :
        m_Swapchain(static_cast<Dx12Swapchain*>(Swapchain)),
        m_QueueManager(Swapchain),
        m_StaticDescriptorHeap{
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SizeOfCPUDescriptor_Resource, false),
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SizeOfCPUDescriptor_Samplers, false),
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SizeOfCPUDescriptor_Rtv, false),
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, SizeOfCPUDescriptor_Dsv, false),
        },
        m_DynamicDescriptorHeap{
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SizeOfGPUDescriptor_Resource, true),
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SizeOfGPUDescriptor_Samplers, true),
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SizeOfGPUDescriptor_Rtv, false),
            Dx12DescriptorHeapBuddyAllocator(Swapchain, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, SizeOfGPUDescriptor_Dsv, false),
        },
        m_ContextPool{
            CommandContextPool(Dx12CommandContextManager(D3D12_COMMAND_LIST_TYPE_DIRECT)),
            CommandContextPool(Dx12CommandContextManager(D3D12_COMMAND_LIST_TYPE_COMPUTE)),
        },
        m_CopyContext(Swapchain)
    {
    }

    //

    Dx12CommandQueueManager* BudgetManager::GetQueueManager()
    {
        return &m_QueueManager;
    }

    //

    void BudgetManager::NewFrame()
    {
        for (auto QueueType : {
                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                 D3D12_COMMAND_LIST_TYPE_COMPUTE })
        {
            auto& Info = *m_QueueManager.Get(QueueType);
            Info.Fence.WaitCPU(m_FenceValue);
        }
        auto& Frame = m_FrameResources[m_FrameIndex];
        Frame.Reset(m_Swapchain);
    }

    void BudgetManager::EndFrame()
    {
        ++m_FenceValue;
        for (auto QueueType : {
                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                 D3D12_COMMAND_LIST_TYPE_COMPUTE })
        {
            auto& [Queue, Fence] = *m_QueueManager.Get(QueueType);
            Fence.SignalGPU(&Queue, m_FenceValue);
        }
        m_FrameIndex = (m_FrameIndex + 1) % m_FrameResources.size();
    }

    //

    void BudgetManager::ResizeFrames(
        size_t FramesCount)
    {
        m_FrameResources.resize(FramesCount);
        m_FrameIndex = 0;
    }

    uint32_t BudgetManager::GetFrameIndex() const
    {
        return m_FrameIndex;
    }

    void BudgetManager::ResetFrameIndex()
    {
        m_FrameIndex = 0;
    }

    void BudgetManager::IdleGPU()
    {
        for (auto QueueType : {
                 D3D12_COMMAND_LIST_TYPE_DIRECT,
             })
        {
            auto& [Queue, Fence] = *m_QueueManager.Get(QueueType);
            Fence.SignalGPU(&Queue, m_FenceValue);
        }

        for (auto QueueType : {
                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                 D3D12_COMMAND_LIST_TYPE_COMPUTE })
        {
            auto& Info = *m_QueueManager.Get(QueueType);
            Info.Fence.WaitCPU(m_FenceValue);
        }

        for (auto& Frame : m_FrameResources)
        {
            Frame.Reset(m_Swapchain);
        }
    }

    void BudgetManager::Shutdown()
    {
        m_CopyContext.Shutdown();
        IdleGPU();
    }

    //

    std::vector<ICommandList*> BudgetManager::AllocateCommandLists(
        D3D12_COMMAND_LIST_TYPE Type,
        size_t                  Count)
    {
        NEON_ASSERT(Type != D3D12_COMMAND_LIST_TYPE_COPY);

        std::vector<ICommandList*> Result;
        Result.reserve(Count);

        auto& Context = m_ContextPool[FrameResource::GetCommandListIndex(Type)];
        auto& Frame   = m_FrameResources[m_FrameIndex];

        std::scoped_lock Lock(Context.Mutex);
        for (size_t i = 0; i < Count; i++)
        {
            Result.emplace_back(Context.Pool.Request(m_Swapchain, Frame));
        }
        return Result;
    }

    void BudgetManager::FreeCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        NEON_ASSERT(Type != D3D12_COMMAND_LIST_TYPE_COPY);
        NEON_ASSERT(!Commands.empty());

        auto& Context = m_ContextPool[FrameResource::GetCommandListIndex(Type)];

        std::scoped_lock Lock(Context.Mutex);
        for (auto Command : Commands)
        {
            Context.Pool.Free(Command);
        }
    }

    void BudgetManager::ResetCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        NEON_ASSERT(!Commands.empty());

        auto& Context = m_ContextPool[FrameResource::GetCommandListIndex(Type)];

        std::scoped_lock Lock(Context.Mutex);

        auto& Frame = m_FrameResources[m_FrameIndex];
        for (auto Command : Commands)
        {
            Context.Pool.Reset(Frame, Command);
        }
    }

    //

    IDescriptorHeapAllocator* BudgetManager::GetDescriptorHeapManager(
        D3D12_DESCRIPTOR_HEAP_TYPE Type,
        bool                       Dynamic)
    {
        return Dynamic ? &m_DynamicDescriptorHeap[Type] : &m_StaticDescriptorHeap[Type];
    }

    void BudgetManager::SafeRelease(
        IDescriptorHeapAllocator*   Allocator,
        const DescriptorHeapHandle& Handle)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[0]);
        auto&            Frame = m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Allocator, Handle);
    }

    void BudgetManager::SafeRelease(
        const Dx12Buffer::Handle& Handle)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[1]);
        auto&            Frame = m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Handle);
    }

    void BudgetManager::SafeRelease(
        const Win32::ComPtr<ID3D12DescriptorHeap>& Descriptor)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[2]);
        auto&            Frame = m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Descriptor);
    }

    void BudgetManager::SafeRelease(
        const Win32::ComPtr<ID3D12Resource>&      Resource,
        const Win32::ComPtr<D3D12MA::Allocation>& Allocation)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[3]);
        auto&            Frame = m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Resource, Allocation);
    }

    void BudgetManager::WaitForCopy(
        Dx12CommandQueue* Queue,
        uint64_t          FenceValue)
    {
        m_CopyContext.WaitForCopy(Queue, FenceValue);
    }

    uint64_t BudgetManager::RequestCopy(
        std::function<void(ICopyCommandList*)> Task)
    {
        return m_CopyContext.EnqueueCopy(Task);
    }
} // namespace Neon::RHI