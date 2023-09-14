#pragma once

#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Frame.hpp>
#include <Private/RHI/Dx12/Fence.hpp>
#include <Allocator/FreeList.hpp>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <future>
#include <latch>

namespace Neon::RHI
{
    class Dx12CommandList;
    class ISwapchain;
    class Dx12Swapchain;

    class Dx12CommandContextManager
    {
    private:
        struct CommandListInstance
        {
            WinAPI::ComPtr<ID3D12GraphicsCommandList> Dx12CmdList;
            std::unique_ptr<Dx12CommandList>          CommandList;

            CommandListInstance(
                ID3D12CommandAllocator* Allocator,
                D3D12_COMMAND_LIST_TYPE CommandType);
        };

        using Dx12CommandListPool = Allocator::FreeList<CommandListInstance>;

    public:
        explicit Dx12CommandContextManager(
            D3D12_COMMAND_LIST_TYPE Type);

        /// <summary>
        /// Request command list from pool
        /// </summary>
        ICommandList* Request(
            ID3D12CommandAllocator* Allocator);

        /// <summary>
        /// Request command list from pool
        /// </summary>
        ICommandList* Request(
            FrameResource& Frame);

        /// <summary>
        /// Free command list to pool
        /// </summary>
        void Free(
            ICommandList* CommandList);

        /// <summary>
        /// Reset command list with new command allocator
        /// </summary>
        void Reset(
            ID3D12CommandAllocator* Allocator,
            ICommandList*           CommandList);

        /// <summary>
        /// Reset command list with new command allocator
        /// </summary>
        void Reset(
            FrameResource& Frame,
            ICommandList*  CommandList);

    private:
        D3D12_COMMAND_LIST_TYPE m_Type;
        Dx12CommandListPool     m_CommandListsPool;

        std::map<ICommandList*, Dx12CommandListPool::Iterator> m_ToPoolMap;
    };

    //

    class CopyContextManager
    {
        using PromiseType = std::promise<uint64_t>;
        using FutureType  = std::future<uint64_t>;

        using PackagedTaskType = std::packaged_task<void(ICommandList*)>;
        struct PackagedTaskResult
        {
            PackagedTaskType Task;
            PromiseType      Promise;
        };

    private:
        static constexpr uint32_t CommandContextCount   = 1;
        static constexpr uint32_t CommandsToHandleCount = 16;

        struct CommandContext
        {
            WinAPI::ComPtr<ID3D12CommandAllocator>    CommandAllocator;
            WinAPI::ComPtr<ID3D12GraphicsCommandList> CommandList;

            CommandContext();
        };

    public:
        CopyContextManager();

        /// <summary>
        /// Enqueue a copy command list to be executed.
        /// </summary>
        uint64_t EnqueueCopy(
            std::function<void(ICommandList*)> Task);

        /// <summary>
        /// Wait for all tasks to finish and shutdown
        /// </summary>
        void Shutdown();

        /// <summary>
        /// Get copy command queue
        /// </summary>
        [[nodiscard]] Dx12CommandQueue* GetQueue() noexcept;

        /// <summary>
        /// Get copy fence
        /// </summary>
        [[nodiscard]] Dx12Fence* GetQueueFence() noexcept;

    private:
        Dx12CommandQueue m_CopyQueue;
        Dx12Fence        m_CopyFence;

        std::array<CommandContext, CommandContextCount> m_CommandContexts;
        std::array<std::jthread, CommandContextCount>   m_Threads;

        uint64_t                       m_CopyId = 1;
        std::mutex                     m_QueueMutex;
        std::queue<PackagedTaskResult> m_Queue;
        std::condition_variable_any    m_TaskWaiter;
    };

    //

    class FrameManager
    {
    public:
        struct CommandContextPool
        {
            Dx12CommandContextManager Pool;
            std::mutex                Mutex;
        };
        using CommandContextPools = std::array<CommandContextPool, 2>;

    public:
        FrameManager();
        NEON_CLASS_NO_COPYMOVE(FrameManager);
        ~FrameManager();

        /// <summary>
        /// Get direct/copy command queue
        /// </summary>
        [[nodiscard]] Dx12CommandQueue* GetQueue(
            bool IsDirect) noexcept;

        /// <summary>
        /// Get direct/copy fence
        /// </summary>
        [[nodiscard]] Dx12Fence* GetQueueFence(
            bool IsDirect) noexcept;

    public:
        /// <summary>
        /// Mark the start of frame.
        /// Wait for previous frame to finish.
        /// </summary>
        void NewFrame();

        /// <summary>
        /// Mark the end of frame.
        /// </summary>
        void EndFrame();

        /// <summary>
        /// Resize frame resources to match swapchain size.
        /// </summary>
        void ResizeFrames(
            size_t FramesCount);

        /// <summary>
        /// Get current frame count
        /// </summary>
        [[nodiscard]] uint32_t GetFrameCount() const;

        /// <summary>
        /// Get current frame index
        /// </summary>
        [[nodiscard]] uint32_t GetFrameIndex() const;

        /// <summary>
        /// Reset frame index to 0
        /// </summary>
        void ResetFrameIndex();

        /// <summary>
        /// Forces GPU to wait for all commands to finish.
        /// </summary>
        void IdleGPU();

    public:
        /// <summary>
        /// Allocate or reuse command lists
        /// </summary>
        [[nodiscard]] std::vector<ICommandList*> AllocateCommandLists(
            D3D12_COMMAND_LIST_TYPE Type,
            size_t                  Count);

        /// <summary>
        /// Free command lists.
        /// </summary>
        void FreeCommandLists(
            D3D12_COMMAND_LIST_TYPE  Type,
            std::span<ICommandList*> Commands);

        /// <summary>
        /// Reset command lists with new command allocator.
        /// </summary>
        void ResetCommandLists(
            D3D12_COMMAND_LIST_TYPE  Type,
            std::span<ICommandList*> Commands);

    public:
        /// <summary>
        /// Enqueue a descriptor handle to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            IDescriptorHeapAllocator*   Allocator,
            const DescriptorHeapHandle& Handle);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const GraphicsMemoryAllocator::Handle& Handle);

        /// <summary>
        /// Enqueue descriptor to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const WinAPI::ComPtr<ID3D12DescriptorHeap>& Resource);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const WinAPI::ComPtr<ID3D12Resource>&      Resource,
            const WinAPI::ComPtr<D3D12MA::Allocation>& Allocation);

        /// <summary>
        /// Enqueue a copy command executed.
        /// </summary>
        uint64_t RequestCopy(
            std::function<void(ICommandList*)> Task);

    public:
        /// <summary>
        /// Get frame descriptor heap allocator
        /// </summary>
        [[nodiscard]] Dx12FrameDescriptorHeap* GetFrameDescriptorAllocator(
            DescriptorType Type) noexcept;

        /// <summary>
        /// Get staged descriptor heap allocator
        /// </summary>
        [[nodiscard]] Dx12StagedDescriptorHeap* GetStagedDescriptorAllocator(
            DescriptorType Type) noexcept;

    private:
        std::vector<UPtr<FrameResource>> m_FrameResources;

        uint32_t m_FrameIndex = 0;

        Dx12CommandQueue m_DirectQueue;
        Dx12Fence        m_DirectFence;

        CommandContextPools m_ContextPool;
        uint64_t            m_FenceValue = 0;

        std::mutex m_StaleResourcesMutex[4];

        CopyContextManager m_CopyContext;
    };
} // namespace Neon::RHI