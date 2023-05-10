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

        ICommandList* AllocateCommandList(
            CommandQueueType Type) override;

        std::vector<ICommandList*> AllocateCommandLists(
            CommandQueueType Type, size_t Count) override;

        void Upload(
            ICommandList* Command) override;

        void Upload(
            const std::vector<ICommandList*>& Commands) override;

    public:
        /// <summary>
        /// Get underlying D3D12 command queue.
        /// </summary>
        [[nodiscard]] ID3D12CommandQueue* Get();

    private:
        Win32::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    };
} // namespace Neon::RHI