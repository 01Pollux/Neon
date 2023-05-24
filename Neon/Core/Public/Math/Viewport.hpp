#pragma once

namespace Neon
{
    struct ViewportF
    {
        float TopLeftX = 0.f;
        float TopLeftY = 0.f;
        float Width    = 0.f;
        float Height   = 0.f;
        float MinDepth = 0.f;
        float MaxDepth = 1.f;
    };
} // namespace Neon
