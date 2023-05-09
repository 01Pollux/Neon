#pragma once

#include <RHI/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12CommandQueue final : public ICommandQueue
    {
    public:
        Dx12CommandQueue(
            CommandQueueType Type);

        /// <summary>
        /// Get underlying D3D12 command queue.
        /// </summary>
        [[nodiscard]] ID3D12CommandQueue* Get();

    private:
        Win32::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    };
} // namespace Neon::RHI