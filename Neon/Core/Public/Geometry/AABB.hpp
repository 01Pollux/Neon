#pragma once

#include <Math/Vector.hpp>
#include <glm/gtx/norm.hpp>

namespace Neon::Geometry
{
    struct AABB
    {
    public:
        Vector3 Min{ std::numeric_limits<float>::max() };
        Vector3 Max{ std::numeric_limits<float>::lowest() };

        /// <summary>
        /// Get the center of the AABB.
        /// </summary>
        [[nodiscard]] Vector3 GetCenter() const noexcept
        {
            return (Min + Max) * 0.5f;
        }

        /// <summary>
        /// Get the size of the AABB.
        /// </summary>
        [[nodiscard]] Vector3 GetSize() const noexcept
        {
            return Max - Min;
        }

        /// <summary>
        /// Get the extents of the AABB.
        /// </summary>
        [[nodiscard]] Vector3 GetExtents() const noexcept
        {
            return GetSize() * 0.5f;
        }

        /// <summary>
        /// Get the volume of the AABB.
        /// </summary>
        [[nodiscard]] float GetVolume() const noexcept
        {
            const auto Size = GetSize();
            return Size.x * Size.y * Size.z;
        }

        /// <summary>
        /// Get the surface area of the AABB.
        /// </summary>
        [[nodiscard]] float GetSurfaceArea() const noexcept
        {
            const auto Size = GetSize();
            return 2.0f * (Size.x * Size.y + Size.x * Size.z + Size.y * Size.z);
        }

        /// <summary>
        /// Get the closest point on the AABB to the given point.
        /// </summary>
        [[nodiscard]] Vector3 GetClosestPoint(
            const Vector3& Point) const noexcept
        {
            return glm::clamp(Point, Min, Max);
        }

        /// <summary>
        /// Get the squared distance from the AABB to the given point.
        /// </summary>
        [[nodiscard]] float GetSquaredDistance(
            const Vector3& Point) const noexcept
        {
            const auto ClosestPoint = GetClosestPoint(Point);
            return glm::distance2(Point, ClosestPoint);
        }

        /// <summary>
        /// Get the distance from the AABB to the given point.
        /// </summary>
        [[nodiscard]] float GetDistance(
            const Vector3& Point) const noexcept
        {
            return glm::sqrt(GetSquaredDistance(Point));
        }

        /// <summary>
        /// Get the squared distance from the AABB to the given AABB.
        /// </summary>
        [[nodiscard]] float GetSquaredDistance(
            const AABB& Other) const noexcept
        {
            const auto ClosestPoint = GetClosestPoint(Other.GetCenter());
            return glm::distance2(Other.GetCenter(), ClosestPoint);
        }

        /// <summary>
        /// Get the distance from the AABB to the given AABB.
        /// </summary>
        [[nodiscard]] float GetDistance(
            const AABB& Other) const noexcept
        {
            return glm::sqrt(GetSquaredDistance(Other));
        }

        /// <summary>
        /// Check if the AABB contains the given point.
        /// </summary>
        [[nodiscard]] bool Contains(
            const Vector3& Point) const noexcept
        {
            return glm::all(glm::lessThanEqual(Min, Point)) &&
                   glm::all(glm::lessThanEqual(Point, Max));
        }

        /// <summary>
        /// Check if the AABB contains the given AABB.
        /// </summary>
        [[nodiscard]] bool Contains(
            const AABB& Other) const noexcept
        {
            return glm::all(glm::lessThanEqual(Min, Other.Min)) &&
                   glm::all(glm::lessThanEqual(Other.Max, Max));
        }

        /// <summary>
        /// Check if the AABB intersects the given AABB.
        /// </summary>
        [[nodiscard]] bool Intersects(
            const AABB& Other) const noexcept
        {
            return glm::all(glm::lessThanEqual(Min, Other.Max)) &&
                   glm::all(glm::lessThanEqual(Other.Min, Max));
        }

        /// <summary>
        /// Expand the AABB to contain the given point.
        /// </summary>
        void Expand(
            const Vector3& Point) noexcept
        {
            Min = glm::min(Min, Point);
            Max = glm::max(Max, Point);
        }

        /// <summary>
        /// Expand the AABB to contain the given AABB.
        /// </summary>
        void Expand(
            const AABB& Other) noexcept
        {
            Min = glm::min(Min, Other.Min);
            Max = glm::max(Max, Other.Max);
        }

    public:
    };
} // namespace Neon::Geometry