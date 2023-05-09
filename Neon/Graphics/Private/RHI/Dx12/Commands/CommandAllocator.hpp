#pragma once

#include <RHI/Commands/CommandAllocator.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12CommandAllocator final : public ICommandAllocator
    {
    public:
        Dx12CommandAllocator(
            CommandQueueType Type);

        void Reset();

    public:
        /// <summary>
        /// Get underlying D3D12 command list.
        /// </summary>
        [[nodiscard]] ID3D12CommandAllocator* Get();

    private:
        Win32::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
    };
} // namespace Neon::RHI