#pragma once

#include <random>

namespace Neon::Random
{
    /// <summary>
    /// Seeds the random number generator.
    /// </summary>
    static void SetSeed(
        size_t Seed)
    {
        extern std::mt19937_64 s_RandomEngine;
        s_RandomEngine.seed(Seed);
    }

    /// <summary>
    /// Gets the random number generator engine.
    /// </summary>
    [[nodiscard]] static std::mt19937_64& GetEngine()
    {
        extern std::mt19937_64 s_RandomEngine;
        return s_RandomEngine;
    }

    //

    template<typename _Ty>
    [[nodiscard]] static _Ty Get()
    {
        return std::uniform_int_distribution<_Ty>()(GetEngine());
    }

    template<typename _Ty>
    [[nodiscard]] static _Ty Get(
        _Ty Min,
        _Ty Max)
    {
        return std::uniform_int_distribution<_Ty>(Min, Max)(GetEngine());
    }
} // namespace Neon::Random