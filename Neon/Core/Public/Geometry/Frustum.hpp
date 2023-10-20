#pragma once

#include <Geometry/Common.hpp>

namespace Neon::Geometry
{
    struct Frustum
    {
        Quaternion Orientation = Vec::Identity<Quaternion>; // Quaternion representing rotation.
        Vector3    Origin      = Vec::Zero<Vector3>;        // Origin of the frustum (and projection).

        float RightSlope  = 1.f;  // Positive X (X/Z)
        float LeftSlope   = -1.f; // Negative X
        float TopSlope    = 1.f;  // Positive Y (Y/Z)
        float BottomSlope = -1.f; // Negative Y
        float Near        = 0.f;  // Z of the near plane.
        float Far         = 1.f;  // Z of the far plane.

        Frustum() = default;

        Frustum(
            const Matrix4x4& InvProjection)
        {
            // Corners of the projection frustum in homogenous space.
            constexpr Vector4 HomogenousPoints[6]{
                { 1.0f, 0.0f, 1.0f, 1.0f },  // right (at far plane)
                { -1.0f, 0.0f, 1.0f, 1.0f }, // left
                { 0.0f, 1.0f, 1.0f, 1.0f },  // top
                { 0.0f, -1.0f, 1.0f, 1.0f }, // bottom
                { 0.0f, 0.0f, 0.0f, 1.0f },  // near
                { 0.0f, 0.0f, 1.0f, 1.0f }   // far
            };

            // Compute the frustum corners in world space.
            Vector4 Points[6]{
                InvProjection * HomogenousPoints[0],
                InvProjection * HomogenousPoints[1],
                InvProjection * HomogenousPoints[2],
                InvProjection * HomogenousPoints[3],
                InvProjection * HomogenousPoints[4],
                InvProjection * HomogenousPoints[5]
            };

            // Compute the slopes.
            RightSlope  = Points[0].x / Points[0].z;
            LeftSlope   = Points[1].x / Points[1].z;
            TopSlope    = Points[2].y / Points[2].z;
            BottomSlope = Points[3].y / Points[3].z;

            // Compute near and far.
            Near = Points[4].z / Points[4].w;
            Far  = Points[5].z / Points[5].w;
        }

    public:
        /// <summary>
        /// Get planes for frustum
        /// </summary>
        [[nodiscard]] std::array<Math::Plane, 6> GetPlanes() const;

    public:
        /// <summary>
        /// Transform the frustum
        /// </summary>
        void Transform(
            const TransformMatrix& Tr)
        {
            Transform(Tr.GetScale(), Tr.GetRotation(), Tr.GetPosition());
        }

        /// <summary>
        /// Transform the frustum
        /// </summary>
        void Transform(
            const Vector3&    Scale,
            const Quaternion& Rotation,
            const Vector3&    Translation);

    public:
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