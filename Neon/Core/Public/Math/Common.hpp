#pragma once

#include <intrin.h>
#include <bit>

// This requires SSE4.2 which is present on Intel Nehalem (Nov. 2008)
// and AMD Bulldozer (Oct. 2011) processors.  I could put a runtime
// check for this, but I'm just going to assume people playing with
// DirectX 12 on Windows 10 have fairly recent machines.
#ifdef _M_X64
#define ENABLE_SSE_CRC32 1
#else
#define ENABLE_SSE_CRC32 0
#endif

#if ENABLE_SSE_CRC32
#pragma intrinsic(_mm_crc32_u32)
#pragma intrinsic(_mm_crc32_u64)
#endif

namespace Math
{
    template<typename _Ty>
    [[nodiscard]] constexpr _Ty AlignUpWithMask(
        _Ty    Value,
        size_t Mask) noexcept
    {
        return _Ty((size_t(Value) + Mask) & ~Mask);
    }

    template<typename _Ty>
    [[nodiscard]] constexpr _Ty AlignDownWithMask(
        _Ty    Value,
        size_t Mask) noexcept
    {
        return _Ty(size_t(Value) & ~Mask);
    }

    template<typename _Ty>
    [[nodiscard]] constexpr _Ty AlignUp(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return AlignUpWithMask(Value, Alignment - 1);
    }

    template<typename _Ty>
    [[nodiscard]] constexpr _Ty AlignDown(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return AlignDownWithMask(Value, Alignment - 1);
    }

    template<typename _Ty>
    [[nodiscard]] constexpr bool IsAligned(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return !(size_t(Value) & (Alignment - 1));
    }

    template<typename _Ty>
    [[nodiscard]] constexpr _Ty DivideByMultiple(
        _Ty    Value,
        size_t Alignment) noexcept
    {
        return _Ty((Value + Alignment - 1) / Alignment);
    }

    template<typename _Ty>
    [[nodiscard]] constexpr bool IsPowerOfTwo(
        _Ty Value) noexcept
    {
        return !(Value & (Value - 1));
    }

    template<typename _Ty>
    [[nodiscard]] constexpr bool IsDivisible(
        _Ty Value,
        _Ty Divisor) noexcept
    {
        return ((Value / Divisor) * Divisor) == Value;
    }
} // namespace Math
