#include <CorePCH.hpp>
#include <Core/BitMask.hpp>
#include <boost/serialization/bitset.hpp>

namespace Neon
{
    template<typename _ETy>
    template<typename _Archive>
    void Bitmask<_ETy>::serialize(
        _Archive& Archive,
        uint32_t)
    {
        Archive& m_Bitset;
    }
} // namespace Neon