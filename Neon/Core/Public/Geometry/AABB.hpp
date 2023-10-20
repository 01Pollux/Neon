#pragma once

#include <Geometry/Common.hpp>

namespace Neon::Geometry
{
    struct AABB
    {
    public:
        Vector3 Center = Vec::Zero<Vector3>;                  // Center of the box.
        Vector3 Extents{ std::numeric_limits<float>::max() }; // Distance from the center to each side.

    public:
        [[nodiscard]] Vector3 Min() const
        {
            return Center - Extents;
        }
        [[nodiscard]] Vector3 Max() const
        {
            return Center + Extents;
        }

    public:
        /// <summary>
        /// Transform the frustum
        /// </summary>
        void Transform(
            float             Scale,
            const Quaternion& Rotation,
            const Vector3&    Translation);

        /// <summary>
        /// Check collision
        /// </summary>
        [[nodiscard]] ContainmentType Contains(
            const AABB& Box) const;

        /// <summary>
        /// Check collision
        /// </summary>
        [[nodiscard]] ContainmentType Contains(
            const Frustum& Fr) const;

        /// <summary>
        /// Check collision
        /// </summary>
        [[nodiscard]] ContainmentType Contains(
            std::span<const Math::Plane> Planes) const;
    };
} // namespace Neon::Geometry