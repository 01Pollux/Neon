#pragma once

#include <cstdint>

namespace Neon::RHI
{
    struct CpuDescriptorHandle
    {
        uint64_t Handle;
    };

    struct GpuDescriptorHandle
    {
        uint64_t Handle;
    };
} // namespace Neon::RHI