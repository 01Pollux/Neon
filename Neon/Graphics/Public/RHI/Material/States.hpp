#pragma once

#include <cstdint>

namespace Neon::RHI::MaterialStates
{
    enum class Sampler : uint8_t
    {
        PointWrap,
        PointClamp,
        LinearWrap,
        LinearClamp,
        AnisotropicWrap,
        AnisotropicClamp,

        _Last
    };

    enum class Blend : uint8_t
    {
        Opaque,
        AlphaBlend,
        Additive,
        NonPremultiplied,
    };

    enum class DepthStencil : uint8_t
    {
        None,
        Default,
        Read,
        ReverseZ,
        ReadReverseZ,
    };

    enum class Rasterizer : uint8_t
    {
        CullNone,
        CullClockwise,
        CullCounterClockwise,
        Wireframe
    };
} // namespace Neon::RHI::MaterialStates