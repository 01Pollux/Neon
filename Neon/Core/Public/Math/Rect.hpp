#pragma once

#include <Math/Vector.hpp>

namespace Neon
{
    template<typename _Ty>
    concept RectType = std::disjunction_v<
        std::is_same<_Ty, Vector2>,
        std::is_same<_Ty, Vector2I>,
        std::is_same<_Ty, Vector2U>>;

    template<RectType _Ty>
    struct RectT
    {
        using value_type = typename _Ty::value_type;

        _Ty Position;
        _Ty Size;

        constexpr RectT(
            const _Ty& Position = {},
            const _Ty& Size     = {}) noexcept :
            Position(Position),
            Size(Size)
        {
        }

        /// <summary>
        /// Get the width of the vector.
        /// </summary>
        [[nodiscard]] constexpr value_type Width() const noexcept
        {
            return Size.x;
        }

        /// <summary>
        /// Set the width of the vector.
        /// </summary>
        [[nodiscard]] constexpr value_type Height() const noexcept
        {
            return Size.y;
        }

        /// <summary>
        /// Get the height of the vector.
        /// </summary>
        [[nodiscard]] constexpr value_type Left() const noexcept
        {
            return Position.x;
        }

        /// <summary>
        /// Set the height of the vector.
        /// </summary>
        [[nodiscard]] constexpr value_type Top() const noexcept
        {
            return Position.y;
        }

        /// <summary>
        /// Get the height of the vector.
        /// </summary>
        [[nodiscard]] constexpr value_type Right() const noexcept
        {
            return Position.x + Width();
        }

        /// <summary>
        /// Set the height of the vector.
        /// </summary>
        [[nodiscard]] constexpr value_type Bottom() const noexcept
        {
            return Position.y + Height();
        }

        //

        /// <summary>
        /// Get the top left corner of the rectangle.
        /// </summary>
        [[nodiscard]] constexpr _Ty TopLeft() const noexcept
        {
            return _Ty(Left(), Top());
        }

        /// <summary>
        /// Get the top right corner of the rectangle.
        /// </summary>
        [[nodiscard]] constexpr _Ty TopRight() const noexcept
        {
            return _Ty(Right(), Top());
        }

        /// <summary>
        /// Get the bottom left corner of the rectangle.
        /// </summary>
        [[nodiscard]] constexpr _Ty BottomLeft() const noexcept
        {
            return _Ty(Left(), Bottom());
        }

        /// <summary>
        /// Get the bottom right corner of the rectangle.
        /// </summary>
        [[nodiscard]] constexpr _Ty BottomRight() const noexcept
        {
            return _Ty(Right(), Bottom());
        }
    };

    using RectI = RectT<Vector2I>;
    using RectF = RectT<Vector2>;
} // namespace Neon
