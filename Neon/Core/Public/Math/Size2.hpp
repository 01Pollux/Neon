#pragma once

#include <Math/Vector.hpp>

namespace Neon
{
    class Size2I final : public Vector2DI
    {
    public:
        using Vector2DI::Vector2DI;
        Size2I(
            int Width,
            int Height) :
            Vector2DI(Width, Height)
        {
        }

        MVECTOR_IMPL_ACCESSOR(0, Width);
        MVECTOR_IMPL_ACCESSOR(1, Height);
    };

    class Size2F final : public Vector2D
    {
    public:
        using Vector2D::Vector2D;
        Size2F(
            float Width,
            float Height) :
            Vector2D(Width, Height)
        {
        }

        MVECTOR_IMPL_ACCESSOR(0, Width);
        MVECTOR_IMPL_ACCESSOR(1, Height);
    };
} // namespace Neon
