#pragma once

#include <RHI/Commands/CommandQueue.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <mutex>

namespace Neon::RHI
{
    class ISwapchain;

    class Dx12CommandQueue final : public ICommandQueue
    {
    public:
        Dx12CommandQueue(
            ISwapchain*      Swapchain,
            CommandQueueType Type);

        std::vector<ICommandList*> AllocateCommandLists(
            CommandQueueType Type, size_t Count) override;

        void FreeCommandLists(
            CommandQueueType         Type,
            std::span<ICommandList*> Commands) override;

        void Upload(
            std::span<ICommandList*> Commands) override;

        void Reset(
            CommandQueueType         Type,
            std::span<ICommandList*> Commands) override;

    public:
        /// <summary>
        /// Get underlying D3D12 command queue.
        /// </summary>
        [[nodiscard]] ID3D12CommandQueue* Get();

    private:
        ISwapchain* m_Swapchain;

        Win32::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    };
} // namespace Neon::RHI