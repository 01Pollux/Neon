#pragma once

#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>
#include <RHI/GlobalBuffer.hpp>

#include <Allocator/Buddy.hpp>
#include <Private/Windows/API/WinPtr.hpp>

#include <mutex>
#include <list>

namespace Neon::RHI
{
    class Dx12ResourceStateManager;

    class GraphicsMemoryAllocator
    {
    public:
        struct Handle
        {
            IBuffer*                      Resource;
            size_t                        Offset;
            size_t                        Size;
            IGlobalBufferPool::BufferType Type;
        };

    private:
        struct BuddyBlock
        {
            Dx12ResourceStateManager& StateManager;
            UPtr<Dx12Buffer>          Buffer;

            Allocator::BuddyAllocator Allocator;

            BuddyBlock(
                Dx12ResourceStateManager&     StateManager,
                D3D12MA::Allocator*           GpuAllocator,
                IGlobalBufferPool::BufferType Type,
                size_t                        SizeOfBuffer);

            ~BuddyBlock();
        };

        struct BufferAllocator
        {
            size_t                SizeOfBuffer = 65'536;
            std::list<BuddyBlock> BufferPools;
        };

        using BufferAllocatorByFlags = std::array<BufferAllocator, size_t(IGlobalBufferPool::BufferType::Count)>;

    public:
        GraphicsMemoryAllocator();

        /// <summary>
        /// Shutdown allocator and free all resources
        /// </summary>
        void Shutdown();

        /// <summary>
        /// Get allocator
        /// </summary>
        [[nodiscard]] D3D12MA::Allocator* GetMA() const;

        /// <summary>
        /// Allocate buffer of size
        /// </summary>
        [[nodiscard]] Handle AllocateBuffer(
            IGlobalBufferPool::BufferType Type,
            size_t                        BufferSize,
            size_t                        Alignement);

        /// <summary>
        /// Free current buffer handle
        /// </summary>
        void FreeBuffers(
            std::span<Handle> Hndl);

    public:
        /// <summary>
        /// Get state manager for this allocator
        /// </summary>
        [[nodiscard]] Dx12ResourceStateManager* GetStateManager();

    private:
        Dx12ResourceStateManager           m_StateManager;
        std::mutex                         m_PoolMutex;
        WinAPI::ComPtr<D3D12MA::Allocator> m_Allocator;
        BufferAllocatorByFlags             m_BufferAllocators;
    };
} // namespace Neon::RHI