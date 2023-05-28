#pragma once

#include <Private/RHI/Dx12/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Frame.hpp>
#include <Private/RHI/Dx12/Fence.hpp>
#include <Allocator/FreeList.hpp>
#include <mutex>

namespace Neon::RHI
{
    class Dx12CommandList;
    class ISwapchain;
    class Dx12Swapchain;

    class Dx12CommandQueueManager
    {
    public:
        struct QueueAndFence
        {
            Dx12CommandQueue Queue;
            Dx12Fence        Fence;

            QueueAndFence(
                ISwapchain*      Swapchain,
                CommandQueueType QueueType);
        };

        explicit Dx12CommandQueueManager(
            ISwapchain* Swapchain);

        /// <summary>
        /// Get graphics command queue
        /// </summary>
        [[nodiscard]] QueueAndFence* GetGraphics();

        /// <summary>
        /// Get graphics compute queue
        /// </summary>
        [[nodiscard]] QueueAndFence* GetCompute();

        /// <summary>
        /// Get graphics copy queue
        /// </summary>
        [[nodiscard]] QueueAndFence* GetCopy();

        /// <summary>
        /// Get queue of type
        /// </summary>
        [[nodiscard]] QueueAndFence* Get(
            D3D12_COMMAND_LIST_TYPE CommandType);

    private:
        QueueAndFence m_Graphics;
        QueueAndFence m_Compute;
        QueueAndFence m_Copy;
    };

    class Dx12CommandContextManager
    {
    private:
        struct CommandListInstance
        {
            Win32::ComPtr<ID3D12GraphicsCommandList> Dx12CmdList;
            std::unique_ptr<Dx12CommandList>         CommandList;

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
            FrameResource& Frame,
            ICommandList*  CommandList);

    private:
        D3D12_COMMAND_LIST_TYPE m_Type;
        Dx12CommandListPool     m_CommandListsPool;

        std::map<ICommandList*, Dx12CommandListPool::Iterator> m_ToPoolMap;
    };

    //

    class BudgetManager
    {
    private:
        struct CommandAllocatorInstance
        {
            Win32::ComPtr<ID3D12CommandAllocator> CommandAllocator;

            CommandAllocatorInstance(
                D3D12_COMMAND_LIST_TYPE CommandType);
        };

        struct CommandListInstance
        {
            Win32::ComPtr<ID3D12GraphicsCommandList> CommandList;

            CommandListInstance(
                ID3D12CommandAllocator* Allocator,
                D3D12_COMMAND_LIST_TYPE CommandType);
        };

    public:
        struct CommandContextPool
        {
            Dx12CommandContextManager Pool;
            std::mutex                Mutex;
        };
        using CommandContextPools = std::array<CommandContextPool, FrameResource::NumCommandContextTypes>;

        using DescriptorHeapAllocators = std::array<Dx12DescriptorHeapBuddyAllocator, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES>;

    public:
        BudgetManager(
            ISwapchain* Swapchain);
        NEON_CLASS_NO_COPYMOVE(BudgetManager);
        ~BudgetManager();

        /// <summary>
        /// Get command queue manager
        /// </summary>
        [[nodiscard]] Dx12CommandQueueManager* GetQueueManager();

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
        /// Get descriptor heap manager
        /// </summary>
        [[nodiscard]] IDescriptorHeapAllocator* GetDescriptorHeapManager(
            D3D12_DESCRIPTOR_HEAP_TYPE Type,
            bool                       Dynamic);

        /// <summary>
        /// Enqueue descriptor heap to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Ptr<IDescriptorHeap>& Heap);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Ptr<IDescriptorHeapAllocator>& Allocator,
            const DescriptorHeapHandle&          Handle);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Dx12Buffer::Handle& Handle);

        /// <summary>
        /// Enqueue resource to be released at the end of the frame.
        /// </summary>
        void SafeRelease(
            const Win32::ComPtr<ID3D12Resource>& Resource);

    private:
        Dx12Swapchain*          m_Swapchain;
        Dx12CommandQueueManager m_QueueManager;

        DescriptorHeapAllocators m_StaticDescriptorHeap;
        DescriptorHeapAllocators m_DynamicDescriptorHeap;

        CommandContextPools m_ContextPool;
        uint64_t            m_FenceValue = 0;

        std::vector<FrameResource> m_FrameResources;
        uint32_t                   m_FrameIndex = 0;

        std::mutex m_StaleResourcesMutex[4];
    };
} // namespace Neon::RHI