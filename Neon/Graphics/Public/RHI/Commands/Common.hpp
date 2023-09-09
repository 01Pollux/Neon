#pragma once

namespace Neon::RHI
{
    enum class CommandQueueType : unsigned char
    {
        Graphics,
        Compute,
        Copy
    };

    enum class CstResourceViewType : uint8_t
    {
        Cbv,
        Srv,
        Uav
    };
} // namespace Neon::RHI
