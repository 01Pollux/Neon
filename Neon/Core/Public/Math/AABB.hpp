#pragma once

#include <Math/Vector.hpp>
#include <glm/gtx/norm.hpp>

namespace Neon::Geometry
{
    template<Vec::VectorType _VecTy>
    struct AABoundingBox
    {
    public:
        using vector_type = _VecTy;
        using value_type  = typename _VecTy::value_type;

        vector_type Min{ std::numeric_limits<float>::max() };
        vector_type Max{ std::numeric_limits<float>::lowest() };

        AABoundingBox() = default;
        AABoundingBox(
            const vector_type& Min,
            const vector_type& Max) :
            Min(Min),
            Max(Max)
        {
        }

        /// <summary>
        /// Get the center of the AABB.
        /// </summary>
        [[nodiscard]] vector_type GetCenter() const noexcept
        {
            return (Min + Max) * 0.5f;
        }

        /// <summary>
        /// Get the size of the AABB.
        /// </summary>
        [[nodiscard]] vector_type GetSize() const noexcept
        {
            return Max - Min;
        }

        /// <summary>
        /// Get the extents of the AABB.
        /// </summary>
        [[nodiscard]] vector_type GetExtents() const noexcept
        {
            return GetSize() * 0.5f;
        }

        /// <summary>
        /// Get the volume of the AABB.
        /// </summary>
        [[nodiscard]] value_type GetVolume() const noexcept
        {
            const auto Size = GetSize();
            return Size.X * Size.Y * Size.Z;
        }

        /// <summary>
        /// Get the surface area of the AABB.
        /// </summary>
        [[nodiscard]] value_type GetSurfaceArea() const noexcept
        {
            const auto Size = GetSize();
            return 2.0f * (Size.X * Size.Y + Size.X * Size.Z + Size.Y * Size.Z);
        }

        /// <summary>
        /// Get the closest point on the AABB to the given point.
        /// </summary>
        [[nodiscard]] vector_type GetClosestPoint(
            const vector_type& Point) const noexcept
        {
            return glm::clamp(Point, Min, Max);
        }

        /// <summary>
        /// Get the squared distance from the AABB to the given point.
        /// </summary>
        [[nodiscard]] value_type GetSquaredDistance(
            const vector_type& Point) const noexcept
        {
            const auto ClosestPoint = GetClosestPoint(Point);
            return glm::distance2(Point, ClosestPoint);
        }

        /// <summary>
        /// Get the distance from the AABB to the given point.
        /// </summary>
        [[nodiscard]] value_type GetDistance(
            const vector_type& Point) const noexcept
        {
            return glm::sqrt(GetSquaredDistance(Point));
        }

        /// <summary>
        /// Get the squared distance from the AABB to the given AABB.
        /// </summary>
        [[nodiscard]] value_type GetSquaredDistance(
            const AABoundingBox& Other) const noexcept
        {
            const auto ClosestPoint = GetClosestPoint(Other.GetCenter());
            return glm::distance2(Other.GetCenter(), ClosestPoint);
        }

        /// <summary>
        /// Get the distance from the AABB to the given AABB.
        /// </summary>
        [[nodiscard]] value_type GetDistance(
            const AABoundingBox& Other) const noexcept
        {
            return glm::sqrt(GetSquaredDistance(Other));
        }

        /// <summary>
        /// Check if the AABB contains the given point.
        /// </summary>
        [[nodiscard]] bool Contains(
            const vector_type& Point) const noexcept
        {
            return glm::all(glm::lessThanEqual(Min, Point)) &&
                   glm::all(glm::lessThanEqual(Point, Max));
        }

        /// <summary>
        /// Check if the AABB contains the given AABB.
        /// </summary>
        [[nodiscard]] bool Contains(
            const AABoundingBox& Other) const noexcept
        {
            return glm::all(glm::lessThanEqual(Min, Other.Min)) &&
                   glm::all(glm::lessThanEqual(Other.Max, Max));
        }

        /// <summary>
        /// Check if the AABB intersects the given AABB.
        /// </summary>
        [[nodiscard]] bool Intersects(
            const AABoundingBox& Other) const noexcept
        {
            return glm::all(glm::lessThanEqual(Min, Other.Max)) &&
                   glm::all(glm::lessThanEqual(Other.Min, Max));
        }

        /// <summary>
        /// Expand the AABB to contain the given point.
        /// </summary>
        void Expand(
            const vector_type& Point) noexcept
        {
            Min = glm::min(Min, Point);
            Max = glm::max(Max, Point);
        }

        /// <summary>
        /// Expand the AABB to contain the given AABB.
        /// </summary>
        void Expand(
            const AABoundingBox& Other) noexcept
        {
            Min = glm::min(Min, Other.Min);
            Max = glm::max(Max, Other.Max);
        }
    };

    using AABoundingBox2D = AABoundingBox<Vector2>;
    using AABoundingBox3D = AABoundingBox<Vector3>;
} // namespace Neon::Geometry