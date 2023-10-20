#pragma once

#include <Math/Vector.hpp>
#include <Math/Matrix.hpp>
#include <glm/gtx/norm.hpp>
#include <span>

namespace Neon::Math
{
    struct Plane : Vector4
    {
    public:
        using Vector4::Vector4;

        /// <summary>
        /// Cast the plane to vector4
        /// </summary>
        Vector4& AsVec4() noexcept
        {
            return static_cast<Vector4&>(*this);
        }

        /// <summary>
        /// Cast the plane to vector4
        /// </summary>
        const Vector4& AsVec4() const noexcept
        {
            return static_cast<const Vector4&>(*this);
        }

        /// <summary>
        /// Cast the plane to vector3
        /// </summary>
        Vector3 AsVec3() const noexcept
        {
            return static_cast<const Vector3&>(*this);
        }

    public:
        /// <summary>
        /// Check intersection between plane and aabb
        /// Center.w needs to be equal to 1.f
        /// </summary>
        void IntersectAxisAlignedBox(
            const Vector4& Center,
            const Vector3& Extents,
            bool&          Outside,
            bool&          Inside) const
        {
            // Compute the distance to the center of the box.
            float Dist = glm::dot(Center, AsVec4());

            // Project the axes of the box onto the normal of the plane.  Half the
            // length of the projection (sometime called the "radius") is equal to
            // h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
            // where h(i) are extents of the box, n is the plane normal, and b(i) are the
            // axes of the box. In this case b(i) = [(1,0,0), (0,1,0), (0,0,1)].
            float Radius = glm::dot(Extents, glm::abs(AsVec3()));

            // Outside the plane?
            Outside = Dist > Radius;

            // Fully inside the plane?
            Inside = Dist < -Radius;
        }

        /// <summary>
        /// Check intersection between plane and frustum plane
        /// </summary>
        void IntersectFrustum(
            std::span<const Vector4> Points,
            bool&                    Outside,
            bool&                    Inside) const
        {
            float Min = std::numeric_limits<float>::max(),
                  Max = std::numeric_limits<float>::lowest();

            Vector3 Vec3 = AsVec3();
            for (auto& Point : Points)
            {
                float Dist = glm::distance(Vec3, Vector3(Point));
                Min        = glm::min(Min, Dist);
                Max        = glm::max(Max, Dist);
            }

            Outside = Min > -w;
            Inside  = Max < -w;
        }

    public:
        /// <summary>
        /// Tranform plane by matrix
        /// </summary>
        void Transform(
            const Matrix4x4& M)
        {
            Vector4 Result = w * M[3];
            Result         = w * M[2] + Result;
            Result         = y * M[1] + Result;
            AsVec4()       = x * M[0] + Result;
        }

        /// <summary>
        /// Tranform plane by matrix
        /// </summary>
        void Transform(
            Quaternion Rotation,
            Vector3    Translation)
        {
            Vector3 Normal = glm::rotate(Rotation, AsVec3());
            float   D      = w - glm::dot(Normal, Translation);
            *this          = Plane(Normal, D);
        }

        /// <summary>
        /// Normalize the plane
        /// </summary>
        void Normalize()
        {
            Vector3 Vec3 = AsVec3();
            AsVec4() /= glm::dot(Vec3, Vec3);
        }
    };
} // namespace Neon::Math