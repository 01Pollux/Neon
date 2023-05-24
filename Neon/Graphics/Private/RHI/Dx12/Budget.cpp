#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Budget.hpp>
#include <Private/RHI/Dx12/Device.hpp>

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
        m_Compute(Swapchain, CommandQueueType::Compute),
        m_Copy(Swapchain, CommandQueueType::Copy)
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

    auto Dx12CommandQueueManager::GetCopy() -> QueueAndFence*
    {
        return &m_Copy;
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
        case D3D12_COMMAND_LIST_TYPE_COPY:
            return &m_Copy;
        default:
            std::unreachable();
        }
    }

    //

    Dx12CommandContextManager::CommandListInstance::CommandListInstance(
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
            CommandList.reset(NEON_NEW Dx12GraphicsCommandList);
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            CommandList.reset(NEON_NEW Dx12ComputeCommandList);
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            CommandList.reset(NEON_NEW Dx12CopyCommandList);
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
        FrameResource& Frame)
    {
        auto Allocator = Frame.RequestAllocator(m_Type);

        auto Iter = m_CommandListsPool.Allocate(Allocator, m_Type);
        ThrowIfFailed(Iter->Dx12CmdList->Reset(Allocator, nullptr));

        Iter->CommandList->AttachCommandList(Iter->Dx12CmdList.Get());
        m_ToPoolMap.emplace(Iter->CommandList.get(), Iter);

        return Iter->CommandList.get();
    }

    void Dx12CommandContextManager::Free(
        ICommandList* CommandList)
    {
        m_CommandListsPool.Free(m_ToPoolMap[CommandList]);
        m_ToPoolMap.erase(CommandList);
    }

    void Dx12CommandContextManager::Reset(
        FrameResource& Frame,
        ICommandList*  CommandList)
    {
        auto Allocator = Frame.RequestAllocator(m_Type);
        ThrowIfFailed(m_ToPoolMap[CommandList]->Dx12CmdList->Reset(Allocator, nullptr));
    }

    //

    BudgetManager::BudgetManager(
        ISwapchain* Swapchain) :
        m_QueueManager(Swapchain),
        m_StaticDescriptorHeap{
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SizeOfCPUDescriptor_Resource, false),
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SizeOfCPUDescriptor_Samplers, false),
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SizeOfCPUDescriptor_Rtv, false),
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, SizeOfCPUDescriptor_Dsv, false),
        },
        m_DynamicDescriptorHeap{
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SizeOfGPUDescriptor_Resource, true),
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SizeOfGPUDescriptor_Samplers, true),
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SizeOfGPUDescriptor_Rtv, false),
            Dx12DescriptorHeapBuddyAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, SizeOfGPUDescriptor_Dsv, false),
        },
        m_ContextPool{
            CommandContextPool(Dx12CommandContextManager(D3D12_COMMAND_LIST_TYPE_DIRECT)),
            CommandContextPool(Dx12CommandContextManager(D3D12_COMMAND_LIST_TYPE_COMPUTE)),
            CommandContextPool(Dx12CommandContextManager(D3D12_COMMAND_LIST_TYPE_COPY)),
        }
    {
    }

    BudgetManager::~BudgetManager()
    {
        IdleGPU();
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
                 D3D12_COMMAND_LIST_TYPE_COMPUTE,
                 D3D12_COMMAND_LIST_TYPE_COPY })
        {
            auto& Info = *m_QueueManager.Get(QueueType);
            Info.Fence.WaitCPU(m_FenceValue);
        }
    }

    void BudgetManager::EndFrame()
    {
        ++m_FenceValue;
        for (auto QueueType : {
                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                 D3D12_COMMAND_LIST_TYPE_COMPUTE,
                 D3D12_COMMAND_LIST_TYPE_COPY })
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

    void BudgetManager::IdleGPU()
    {
        for (auto QueueType : {
                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                 D3D12_COMMAND_LIST_TYPE_COMPUTE,
                 D3D12_COMMAND_LIST_TYPE_COPY })
        {
            auto& [Queue, Fence] = *m_QueueManager.Get(QueueType);
            Fence.SignalGPU(&Queue, m_FenceValue);
        }

        for (auto QueueType : {
                 D3D12_COMMAND_LIST_TYPE_DIRECT,
                 D3D12_COMMAND_LIST_TYPE_COMPUTE,
                 D3D12_COMMAND_LIST_TYPE_COPY })
        {
            auto& Info = *m_QueueManager.Get(QueueType);
            Info.Fence.WaitCPU(m_FenceValue);
        }
    }

    //

    std::vector<ICommandList*> BudgetManager::AllocateCommandLists(
        D3D12_COMMAND_LIST_TYPE Type,
        size_t                  Count)
    {
        std::vector<ICommandList*> Result;
        Result.reserve(Count);

        auto&           Context = m_ContextPool[FrameResource::GetCommandListIndex(Type)];
        std::lock_guard Lock(Context.Mutex);

        auto& Frame = m_FrameResources[m_FrameIndex];
        for (size_t i = 0; i < Count; i++)
        {
            Result.emplace_back(Context.Pool.Request(Frame));
        }
        return Result;
    }

    void BudgetManager::FreeCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        NEON_ASSERT(!Commands.empty());

        auto&           Context = m_ContextPool[FrameResource::GetCommandListIndex(Type)];
        std::lock_guard Lock(Context.Mutex);

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

        auto&           Context = m_ContextPool[FrameResource::GetCommandListIndex(Type)];
        std::lock_guard Lock(Context.Mutex);

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

} // namespace Neon::RHI