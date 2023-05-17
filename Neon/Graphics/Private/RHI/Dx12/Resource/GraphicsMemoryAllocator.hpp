#pragma once

#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/Resource/State.hpp>

#include <Allocator/Buddy.hpp>
#include <Window/Win32/WinPtr.hpp>

#include <mutex>
#include <list>

namespace Neon::RHI
{
    class Dx12ResourceStateManager;

    enum class GraphicsBufferType : uint8_t
    {
        Default,
        Upload,
        Readback,

        Count
    };

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
            std::mutex PoolMutex;

            size_t                SizeOfBuffer = 65'536;
            std::list<BuddyBlock> BufferPools;
        };

        using BufferAllocatorsType   = std::array<BufferAllocator, size_t(GraphicsBufferType::Count)>;
        using BufferAllocatorByFlags = std::map<D3D12_RESOURCE_FLAGS, BufferAllocatorsType>;

    public:
        struct Handle
        {
            Win32::ComPtr<ID3D12Resource> Resource;
            size_t                        Offset;
            size_t                        Size;
            GraphicsBufferType            Type;
            D3D12_RESOURCE_FLAGS          Flags;
        };

        /// <summary>
        /// Initialize allocator
        /// </summary>
        void Initialize(
            const D3D12MA::ALLOCATOR_DESC& Desc);

        /// <summary>
        /// Get allocator
        /// </summary>
        [[nodiscard]] D3D12MA::Allocator* GetMA() const;

        /// <summary>
        /// Allocate buffer of size
        /// </summary>
        [[nodiscard]] Handle AllocateBuffer(
            GraphicsBufferType   Type,
            size_t               BufferSize,
            size_t               Alignement,
            D3D12_RESOURCE_FLAGS Flags);

        /// <summary>
        /// Free current buffer handle
        /// </summary>
        void FreeBuffer(
            const Handle& Data);

    public:
        /// <summary>
        /// Get state manager for this allocator
        /// </summary>
        [[nodiscard]] Dx12ResourceStateManager* GetStateManager();

    private:
        Dx12ResourceStateManager          m_StateManager;
        Win32::ComPtr<D3D12MA::Allocator> m_Allocator;
        BufferAllocatorByFlags            m_BufferAllocators;
    };
} // namespace Neon::RHI