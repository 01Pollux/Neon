#pragma once

#include <Geometry/Common.hpp>

namespace Neon::Geometry
{
    struct Ray
    {
        Vector3 Origin;
        Vector3 Direction;

        Ray() = default;
        Ray(const Vector3& origin, const Vector3& direction) :
            Origin(origin), Direction(direction)
        {
        }

    public:
        /// <summary>
        /// Returns a point along the ray at a distance t from the origin.
        /// </summary>
        [[nodiscard]] Vector3 PointAt(
            float t) const
        {
            return Origin + t * Direction;
        }

    public:
        /// <summary>
        /// Returns a ray that has been transformed by the given matrix.
        /// </summary>
        [[nodiscard]] Ray Transform(
            const Matrix4x4& Mat) const
        {
            return Ray(
                Vector3(Mat * Vector4(Origin, 1.0f)),
                Vector3(Mat * Vector4(Direction, 0.0f)));
        }

        /// <summary>
        /// Check intersection between two lines.
        /// Disjoint are parallel
        /// </summary>
        [[nodiscard]] ContainmentType Contains(
            const Ray& Other,
            float*     T = nullptr,
            float*     S = nullptr)
        {
            Vector3 V1V2 = glm::cross(Other.Origin, Origin);
            float   Dot2 = glm::pow(glm::dot(V1V2, V1V2), 2.f);

            if (!Dot2)
            {
                return ContainmentType::Disjoint;
            }

            if (T)
            {
                Matrix3x3 M(Other.Origin - Origin, Other.Direction, V1V2);
                *T = glm::determinant(M) / Dot2;
            }

            if (S)
            {
                Matrix3x3 M(Other.Origin - Origin, Direction, V1V2);
                *S = glm::determinant(M) / Dot2;
            }

            return ContainmentType::Intersects;
        }
    };
} // namespace Neon::Geometry