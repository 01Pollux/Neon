#pragma once

#include <Math/Plane.hpp>

namespace Neon::Geometry
{
    static constexpr Vector3 s_BoxOffset[8]{
        { -1.0f, -1.0f, 1.0f },
        { 1.0f, -1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { -1.0f, 1.0f, 1.0f },
        { -1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, -1.0f },
        { 1.0f, 1.0f, -1.0f },
        { -1.0f, 1.0f, -1.0f }
    };

    //

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

    //

    struct AABB;
    struct Frustum;
} // namespace Neon::Geometry