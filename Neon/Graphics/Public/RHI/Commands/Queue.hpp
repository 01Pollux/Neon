#pragma once

#include <span>
#include <RHI/Commands/Common.hpp>

namespace Neon::RHI
{
    class ISwapchain;
    class ICommandList;

    class ICommandQueue
    {
    public:
        [[nodiscard]] static ICommandQueue* Create(
            ISwapchain*      Swapchain,
            CommandQueueType Type);

        virtual ~ICommandQueue() = default;

        /// <summary>
        /// Allocate group of command lists in advance.
        /// </summary>
        virtual std::vector<ICommandList*> AllocateCommandLists(
            CommandQueueType Type,
            size_t           Count) = 0;

        /// <summary>
        /// Free group of command lists.
        /// </summary>
        virtual void FreeCommandLists(
            CommandQueueType         Type,
            std::span<ICommandList*> Commands) = 0;

        /// <summary>
        /// Submit command lists to the GPU.
        /// </summary>
        virtual void Upload(
            std::span<ICommandList*> Commands) = 0;

        /// <summary>
        /// Reset the command list.
        /// </summary>
        virtual void Reset(
            CommandQueueType         Type,
            std::span<ICommandList*> Commands) = 0;
    };
} // namespace Neon::RHI
