#pragma once

#include <type_traits>

namespace Neon::Math
{
    template<std::integral _Ty>
    [[nodiscard]] constexpr _Ty AlignUpWithMask(
        _Ty    Value,
        size_t Mask) noexcept
    {
        return _Ty((size_t(Value) + Mask) & ~Mask);
    }

    template<std::integral _Ty>
    [[nodiscard]] constexpr _Ty AlignDownWithMask(
        _Ty    Value,
        size_t Mask) noexcept
    {
        return _Ty(size_t(Value) & ~Mask);
    }

    template<std::integral _Ty>
    [[nodiscard]] constexpr _Ty AlignUp(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return AlignUpWithMask(Value, Alignment - 1);
    }

    template<std::integral _Ty>
    [[nodiscard]] constexpr _Ty AlignDown(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return AlignDownWithMask(Value, Alignment - 1);
    }

    template<std::integral _Ty>
    [[nodiscard]] constexpr bool IsAligned(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return !(size_t(Value) & (Alignment - 1));
    }

    template<std::integral _Ty>
    [[nodiscard]] constexpr _Ty DivideByMultiple(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return _Ty((Value + Alignment - 1) / Alignment);
    }

    template<std::integral _Ty>
    [[nodiscard]] constexpr bool IsPowerOfTwo(
        _Ty Value) noexcept
    {
        return !(Value & (Value - 1));
    }

    template<std::integral _Ty>
    [[nodiscard]] constexpr bool IsDivisible(
        _Ty Value,
        _Ty Divisor) noexcept
    {
        return ((Value / Divisor) * Divisor) == Value;
    }
} // namespace Neon::Math
