#pragma once

#include <Math/Vector.hpp>
#include <Math/Matrix.hpp>

namespace Neon::Geometry
{
    enum class ContainmentType : uint8_t
    {
        Disjoint,
        Intersects,
        Contains
    };

    enum class PlaneIntersectionType : uint8_t
    {
        Front,
        Intersecting,
        Back
    };
} // namespace Neon::Geometry