#pragma once

#include <Math/Vector.hpp>
#include <glm/gtx/norm.hpp>

namespace Neon::Geometry
{
    struct Plane;
    struct Sphere;
    struct Cone;
    struct Cylinder;
    struct Box;

    struct Sphere
    {
        Vector3 Center{};
        float   Radius{};
    };

    struct Cone
    {
        Vector3 Tip{};
        float   Height{};
        Vector3 Distanec{};
        float   Radius{};
    };

    struct Cylinder
    {
        Vector3 Center{};
        float   Height{};
        Vector3 Distance{};
        float   Radius{};
    };

    struct Plane
    {
        Vector3 Normal{};
        float   Distance{};

        constexpr Plane() = default;
        Plane(
            const Vector3& Pt0,
            const Vector3& Pt1,
            const Vector3& Pt2) :
            Normal(glm::normalize(glm::cross(Pt1 - Pt0, Pt2 - Pt0))),
            Distance(glm::dot(Normal, Pt0))
        {
        }
    };

    struct Box
    {
        Vector3 Center{};
        Vector3 Extents{};
    };
} // namespace Neon::Geometry