#pragma once

#include <span>
#include <RHI/Commands/Commands.hpp>

namespace Neon::RHI
{
    class ICommandAllocator;

    class ICommandList
    {
    public:
        [[nodiscard]] static ICommandList* Create(
            ICommandAllocator* Allocator,
            CommandQueueType   Type);

        virtual ~ICommandList() = default;

        /// <summary>
        /// Reset command list.
        /// </summary>
        virtual void Reset(
            ICommandAllocator* Allocator) = 0;
    };
} // namespace Neon::RHI
