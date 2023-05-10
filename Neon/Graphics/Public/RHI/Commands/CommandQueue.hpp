#pragma once

#include <span>
#include <RHI/Commands/Commands.hpp>

namespace Neon::RHI
{
    class ICommandList;

    class ICommandQueue
    {
    public:
        [[nodiscard]] static ICommandQueue* Create(
            CommandQueueType Type);

        virtual ~ICommandQueue() = default;

        /// <summary>
        /// Allocate a command list.
        /// </summary>
        virtual ICommandList* AllocateCommandList(
            CommandQueueType Type) = 0;

        /// <summary>
        /// Allocate group of command lists in advance.
        /// </summary>
        virtual std::vector<ICommandList*> AllocateCommandLists(
            CommandQueueType Type,
            size_t           Count) = 0;

        /// <summary>
        /// Submit a command list to the GPU.
        /// </summary>
        virtual void Upload(
            ICommandList* Command) = 0;

        /// <summary>
        /// Submit command lists to the GPU.
        /// </summary>
        virtual void Upload(
            const std::vector<ICommandList*>& Commands) = 0;
    };
} // namespace Neon::RHI
