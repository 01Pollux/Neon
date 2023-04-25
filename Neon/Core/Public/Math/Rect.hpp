#pragma once

#include <Math/Vector.hpp>

namespace Neon
{
    template<typename _Ty>
    struct MRect
    {
        using value_type = typename _Ty::value_type;

        _Ty Position;
        _Ty Size;

        constexpr MRect(
            const _Ty& Position = {},
            const _Ty& Size     = {}) noexcept :
            Position(Position),
            Size(Size)
        {
        }

        [[nodiscard]] constexpr value_type Width() const noexcept
        {
            return Size.x();
        }

        [[nodiscard]] constexpr value_type Height() const noexcept
        {
            return Size.y();
        }

        [[nodiscard]] constexpr value_type Left() const noexcept
        {
            return Position.x();
        }

        [[nodiscard]] constexpr value_type Top() const noexcept
        {
            return Position.y();
        }

        [[nodiscard]] constexpr value_type Right() const noexcept
        {
            return Position.x() + Width();
        }

        [[nodiscard]] constexpr value_type Bottom() const noexcept
        {
            return Position.y() + Height();
        }

        //

        [[nodiscard]] constexpr _Ty TopLeft() const noexcept
        {
            return _Ty(Left(), Top());
        }

        [[nodiscard]] constexpr _Ty TopRight() const noexcept
        {
            return _Ty(Right(), Top());
        }

        [[nodiscard]] constexpr _Ty BottomLeft() const noexcept
        {
            return _Ty(Left(), Bottom());
        }

        [[nodiscard]] constexpr _Ty BottomRight() const noexcept
        {
            return _Ty(Right(), Bottom());
        }
    };

    using RectI = MRect<Vector2DI>;
    using RectF = MRect<Vector2D>;
} // namespace Neon
