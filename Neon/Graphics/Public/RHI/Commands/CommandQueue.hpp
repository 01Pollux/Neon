#pragma once

#include <span>
#include <RHI/Commands/Commands.hpp>

namespace Neon::RHI
{
    class ICommandQueue
    {
    public:
        [[nodiscard]] static ICommandQueue* Create(
            CommandQueueType Type);

        virtual ~ICommandQueue() = default;
    };
} // namespace Neon::RHI
