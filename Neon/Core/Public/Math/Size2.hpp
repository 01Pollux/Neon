#pragma once

#include <Math/Vector.hpp>

namespace Neon
{
    template<typename _Ty>
    concept Size2Type = std::disjunction_v<
        std::is_same<_Ty, Vector2>,
        std::is_same<_Ty, Vector2I>,
        std::is_same<_Ty, Vector2U>>;

    template<Size2Type _Ty>
    struct Size2T : _Ty
    {
    public:
        using _Ty::_Ty;

        using value_type = typename _Ty::value_type;

        /// <summary>
        /// Get the width of the vector.
        /// </summary>
        [[nodiscard]] value_type Width() const
        {
            return this->x;
        }

        /// <summary>
        /// Set the width of the vector.
        /// </summary>
        [[nodiscard]] void Width(
            value_type Val)
        {
            this->x = Val;
        }

        /// <summary>
        /// Get the height of the vector.
        /// </summary>
        [[nodiscard]] value_type Height() const
        {
            return this->y;
        }

        /// <summary>
        /// Set the height of the vector.
        /// </summary>
        [[nodiscard]] void Height(
            value_type Val)
        {
            this->y = Val;
        }
    };

    using Size2  = Size2T<Vector2>;
    using Size2I = Size2T<Vector2I>;
    using Size2U = Size2T<Vector2U>;
} // namespace Neon
