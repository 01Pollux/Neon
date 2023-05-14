#pragma once

#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Allocator/FreeList.hpp>

namespace Neon::RHI
{
    class FrameResource
    {
    public:
        static constexpr size_t NumCommandContextTypes = 3;

    private:
        struct CommandAllocatorInstance
        {
            Win32::ComPtr<ID3D12CommandAllocator> CommandAllocator;

            CommandAllocatorInstance(
                D3D12_COMMAND_LIST_TYPE CommandType);
        };
        using Dx12CommandAllocatorPool  = Allocator::FreeList<CommandAllocatorInstance>;
        using Dx12CommandAllocatorPools = std::array<Dx12CommandAllocatorPool, NumCommandContextTypes>;

    public:
        /// <summary>
        /// Request command allocator from pool
        /// </summary>
        ID3D12CommandAllocator* RequestAllocator(
            D3D12_COMMAND_LIST_TYPE CommandType);

    public:
        /// <summary>
        /// Convert index to command list type
        /// </summary>
        static D3D12_COMMAND_LIST_TYPE GetCommandListType(
            size_t Index) noexcept;

        /// <summary>
        /// Convert command list type to index
        /// </summary>
        static size_t GetCommandListIndex(
            D3D12_COMMAND_LIST_TYPE Type) noexcept;

    private:
        Dx12CommandAllocatorPools m_AllocatorsPools;
    };
} // namespace Neon::RHI