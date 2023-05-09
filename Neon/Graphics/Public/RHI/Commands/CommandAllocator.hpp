#pragma once

#include <span>
#include <RHI/Commands/Commands.hpp>

namespace Neon::RHI
{
    class ICommandAllocator
    {
    public:
        [[nodiscard]] static ICommandAllocator* Create(
            CommandQueueType Type);

        virtual ~ICommandAllocator() = default;

        /// <summary>
        /// Reset command allocator.
        /// </summary>
        virtual void Reset() = 0;
    };
} // namespace Neon::RHI
