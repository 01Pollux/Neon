#pragma once

namespace Neon::RHI
{
    enum class CommandQueueType : unsigned char
    {
        Graphics,
        Compute,
        Copy
    };
} // namespace Neon::RHI
