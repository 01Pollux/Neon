#pragma once

#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>
#include <Private/RHI/Dx12/Resource/Resource.hpp>

#include <Allocator/Buddy.hpp>
#include <Window/Win32/WinPtr.hpp>

#include <mutex>
#include <list>

namespace Neon::RHI
{
    class Dx12ResourceStateManager;

    class GraphicsMemoryAllocator
    {
        struct BuddyBlock
        {
            Dx12ResourceStateManager&          StateManager;
            Win32::ComPtr<ID3D12Resource>      Resource;
            Win32::ComPtr<D3D12MA::Allocation> Allocation;

            Allocator::BuddyAllocator Allocator;

            BuddyBlock(
                Dx12ResourceStateManager& StateManager,
                D3D12MA::Allocator*       GpuAllocator,
                GraphicsBufferType        Type,
                D3D12_RESOURCE_FLAGS      Flags,
                size_t                    SizeOfBuffer);

            NEON_CLASS_NO_COPY(BuddyBlock);
            NEON_CLASS_NO_MOVE(BuddyBlock);
            ~BuddyBlock();
        };

        struct BufferAllocator
        {
            size_t                SizeOfBuffer = 65'536;
            std::list<BuddyBlock> BufferPools;
        };

        using BufferAllocatorsType   = std::array<BufferAllocator, size_t(GraphicsBufferType::Count)>;
        using BufferAllocatorByFlags = std::map<D3D12_RESOURCE_FLAGS, BufferAllocatorsType>;

    public:
        GraphicsMemoryAllocator();

        /// <summary>
        /// Get allocator
        /// </summary>
        [[nodiscard]] D3D12MA::Allocator* GetMA() const;

        /// <summary>
        /// Allocate buffer of size
        /// </summary>
        [[nodiscard]] Dx12Buffer::Handle AllocateBuffer(
            GraphicsBufferType   Type,
            size_t               BufferSize,
            size_t               Alignement,
            D3D12_RESOURCE_FLAGS Flags);

        /// <summary>
        /// Free current buffer handle
        /// </summary>
        void FreeBuffers(
            std::span<Dx12Buffer::Handle> Handles);

    public:
        /// <summary>
        /// Get state manager for this allocator
        /// </summary>
        [[nodiscard]] Dx12ResourceStateManager* GetStateManager();

    private:
        Dx12ResourceStateManager          m_StateManager;
        std::mutex                        m_PoolMutex;
        Win32::ComPtr<D3D12MA::Allocator> m_Allocator;
        BufferAllocatorByFlags            m_BufferAllocators;
    };
} // namespace Neon::RHI