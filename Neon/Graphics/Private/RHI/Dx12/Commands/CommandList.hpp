#pragma once

#include <RHI/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12CommandList final : public ICommandList
    {
    public:
        Dx12CommandList(
            ICommandAllocator* Allocator,
            CommandQueueType   Type);

        void Reset(
            ICommandAllocator* Allocator);

    public:
        /// <summary>
        /// Get underlying D3D12 command list.
        /// </summary>
        [[nodiscard]] ID3D12GraphicsCommandList* Get();

    private:
        Win32::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    };
} // namespace Neon::RHI