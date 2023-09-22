#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/FrameManager.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Log/Logger.hpp>

namespace Neon::RHI
{
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
        CommandList.reset(NEON_NEW Dx12CommandList);
    }

    Dx12CommandContextManager::Dx12CommandContextManager(
        D3D12_COMMAND_LIST_TYPE Type) :
        m_Type(Type)
    {
    }

    ICommandList* Dx12CommandContextManager::Request(
        ID3D12CommandAllocator* Allocator)
    {
        auto Iter = m_CommandListsPool.Allocate(Allocator, m_Type);
        ThrowIfFailed(Iter->Dx12CmdList->Reset(Allocator, nullptr));

        ID3D12DescriptorHeap* Heaps[]{
            static_cast<Dx12FrameDescriptorHeap*>(IFrameDescriptorHeap::Get(DescriptorType::ResourceView))->GetHeap(),
            static_cast<Dx12FrameDescriptorHeap*>(IFrameDescriptorHeap::Get(DescriptorType::Sampler))->GetHeap()
        };
        Iter->Dx12CmdList->SetDescriptorHeaps(2, Heaps);

        Iter->CommandList->AttachCommandList(Iter->Dx12CmdList.Get());
        m_ToPoolMap.emplace(Iter->CommandList.get(), Iter);

        static_cast<Dx12CommandList*>(Iter->CommandList.get())->Reset();

        return Iter->CommandList.get();
    }

    ICommandList* Dx12CommandContextManager::Request(
        Dx12FrameResource& Frame)
    {
        return Request(Frame.RequestAllocator(m_Type));
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
        auto& CommandData = m_ToPoolMap[CommandList];
        ThrowIfFailed(CommandData->Dx12CmdList->Reset(Allocator, nullptr));

        ID3D12DescriptorHeap* Heaps[]{
            static_cast<Dx12FrameDescriptorHeap*>(IFrameDescriptorHeap::Get(DescriptorType::ResourceView))->GetHeap(),
            static_cast<Dx12FrameDescriptorHeap*>(IFrameDescriptorHeap::Get(DescriptorType::Sampler))->GetHeap()
        };
        CommandData->Dx12CmdList->SetDescriptorHeaps(2, Heaps);

        static_cast<Dx12CommandList*>(CommandData->CommandList.get())->Reset();
    }

    void Dx12CommandContextManager::Reset(
        Dx12FrameResource& Frame,
        ICommandList*      CommandList)
    {
        return Reset(Frame.RequestAllocator(m_Type), CommandList);
    }

    //

    FrameManager::FrameManager() :
        m_DirectQueue(RHI::CommandQueueType::Graphics),
        m_DirectFence(0),
        m_ContextPool{
            CommandContextPool(Dx12CommandContextManager(D3D12_COMMAND_LIST_TYPE_DIRECT)),
            CommandContextPool(Dx12CommandContextManager(D3D12_COMMAND_LIST_TYPE_COMPUTE))
        }
    {
        RHI::RenameObject(m_DirectQueue.Get(), STR("Main Command Queue"));
    }

    FrameManager::~FrameManager()
    {
        m_CopyContext.Shutdown();
        IdleGPU();
    }

    Dx12CommandQueue* FrameManager::GetQueue(
        bool IsDirect) noexcept
    {
        return IsDirect ? &m_DirectQueue : m_CopyContext.GetQueue();
    }

    Dx12Fence* FrameManager::GetQueueFence(
        bool IsDirect) noexcept
    {
        return IsDirect ? &m_DirectFence : m_CopyContext.GetQueueFence();
    }

    //

    void FrameManager::NewFrame()
    {
        m_DirectFence.WaitCPU(m_FenceValue);
        auto& Frame = *m_FrameResources[m_FrameIndex];
        Frame.Reset();
    }

    void FrameManager::EndFrame()
    {
        ++m_FenceValue;
        m_DirectFence.SignalGPU(&m_DirectQueue, m_FenceValue);
        m_FrameIndex = (m_FrameIndex + 1) % uint32_t(m_FrameResources.size());
    }

    //

    void FrameManager::ResizeFrames(
        size_t FramesCount)
    {
        auto OldSize = m_FrameResources.size();
        m_FrameResources.resize(FramesCount);
        for (size_t i = OldSize; i < FramesCount; i++)
        {
            m_FrameResources[i] = std::make_unique<Dx12FrameResource>();
        }
        m_FrameIndex = 0;
    }

    uint32_t FrameManager::GetFrameCount() const
    {
        return uint32_t(m_FrameResources.size());
    }

    uint32_t FrameManager::GetFrameIndex() const
    {
        return m_FrameIndex;
    }

    void FrameManager::ResetFrameIndex()
    {
        m_FrameIndex = 0;
    }

    void FrameManager::IdleGPU()
    {
        m_DirectFence.SignalGPU(&m_DirectQueue, m_FenceValue);
        m_DirectFence.WaitCPU(m_FenceValue);

        for (auto& Frame : m_FrameResources)
        {
            Frame->Reset();
        }
    }

    //

    std::vector<ICommandList*> FrameManager::AllocateCommandLists(
        D3D12_COMMAND_LIST_TYPE Type,
        size_t                  Count)
    {
        NEON_ASSERT(Type != D3D12_COMMAND_LIST_TYPE_COPY);
        NEON_ASSERT(Count);

        std::vector<ICommandList*> Result;
        Result.reserve(Count);

        auto& Context = m_ContextPool[Dx12FrameResource::GetCommandListIndex(Type)];
        auto& Frame   = *m_FrameResources[m_FrameIndex];

        std::scoped_lock Lock(Context.Mutex);
        for (size_t i = 0; i < Count; i++)
        {
            Result.emplace_back(Context.Pool.Request(Frame));
        }
        return Result;
    }

    void FrameManager::FreeCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        NEON_ASSERT(Type != D3D12_COMMAND_LIST_TYPE_COPY);
        NEON_ASSERT(!Commands.empty());

        auto& Context = m_ContextPool[Dx12FrameResource::GetCommandListIndex(Type)];

        std::scoped_lock Lock(Context.Mutex);
        for (auto Command : Commands)
        {
            Context.Pool.Free(Command);
        }
    }

    void FrameManager::ResetCommandLists(
        D3D12_COMMAND_LIST_TYPE  Type,
        std::span<ICommandList*> Commands)
    {
        NEON_ASSERT(!Commands.empty());
        NEON_ASSERT(!Commands.empty());

        auto& Context = m_ContextPool[Dx12FrameResource::GetCommandListIndex(Type)];

        std::scoped_lock Lock(Context.Mutex);

        auto& Frame = *m_FrameResources[m_FrameIndex];
        for (auto Command : Commands)
        {
            Context.Pool.Reset(Frame, Command);
        }
    }

    //

    void FrameManager::SafeRelease(
        IDescriptorHeapAllocator*   Allocator,
        const DescriptorHeapHandle& Handle)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[0]);
        auto&            Frame = *m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Allocator, Handle);
    }

    void FrameManager::SafeRelease(
        const GraphicsMemoryAllocator::Handle& Handle)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[1]);
        auto&            Frame = *m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Handle);
    }

    void FrameManager::SafeRelease(
        const WinAPI::ComPtr<ID3D12DescriptorHeap>& Descriptor)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[2]);
        auto&            Frame = *m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Descriptor);
    }

    void FrameManager::SafeRelease(
        const WinAPI::ComPtr<ID3D12Resource>&      Resource,
        const WinAPI::ComPtr<D3D12MA::Allocation>& Allocation)
    {
        std::scoped_lock Lock(m_StaleResourcesMutex[3]);
        auto&            Frame = *m_FrameResources[m_FrameIndex];
        Frame.SafeRelease(Resource, Allocation);
    }

    std::future<void> FrameManager::RequestCopy(
        std::move_only_function<void(ICommandList*)> CopyTask,
        std::move_only_function<void()>              PostCopyTask)
    {
        return m_CopyContext.EnqueueCopy(std::move(CopyTask), std::move(PostCopyTask));
    }

    Dx12FrameDescriptorHeap* FrameManager::GetFrameDescriptorAllocator(
        DescriptorType Type) noexcept
    {
        auto& Frame = *m_FrameResources[m_FrameIndex];
        return Frame.GetFrameDescriptorAllocator(Type);
    }

    Dx12StagedDescriptorHeap* FrameManager::GetStagedDescriptorAllocator(
        DescriptorType Type) noexcept
    {
        auto& Frame = *m_FrameResources[m_FrameIndex];
        return Frame.GetStagedDescriptorAllocator(Type);
    }
} // namespace Neon::RHI