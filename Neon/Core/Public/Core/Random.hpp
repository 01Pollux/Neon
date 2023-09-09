#pragma once

#include <random>

namespace Neon::Random
{
    namespace Impl
    {
        static std::mt19937_64 s_RandomEngine;
    }

    /// <summary>
    /// Seeds the random number generator.
    /// </summary>
    void SetSeed(
        size_t Seed)
    {
        Impl::s_RandomEngine.seed(Seed);
    }

    /// <summary>
    /// Gets the random number generator engine.
    /// </summary>
    [[nodiscard]] std::mt19937_64& GetEngine()
    {
        return Impl::s_RandomEngine;
    }

    //

    template<typename _Ty>
    _Ty Get()
    {
        return std::uniform_int_distribution<_Ty>()(Impl::s_RandomEngine);
    }

    template<typename _Ty>
    _Ty Get(
        _Ty Min,
        _Ty Max)
    {
        return std::uniform_int_distribution<_Ty>(Min, Max)(Impl::s_RandomEngine);
    }
} // namespace Neon::Random