#include <CorePCH.hpp>
#include <Geometry/AABB.hpp>
#include <Geometry/Frustum.hpp>

namespace Neon::Geometry
{
    std::array<Math::Plane, 6> Frustum::GetPlanes() const
    {
        // Create 6 planes
        std::array Planes{
            Math::Plane(0.0f, 0.0f, -1.0f, Near),
            Math::Plane(0.0f, 0.0f, 1.0f, -Far),
            Math::Plane(1.0f, 0.0f, -RightSlope, 0.0f),
            Math::Plane(-1.0f, 0.0f, LeftSlope, 0.0f),
            Math::Plane(0.0f, 1.0f, -TopSlope, 0.0f),
            Math::Plane(0.0f, -1.0f, BottomSlope, 0.0f)
        };
        for (auto& Plane : Planes)
        {
            Plane.Transform(Orientation, Origin);
            Plane.Normalize();
        }
        return Planes;
    }

    //

    void Frustum::Transform(
        float             Scale,
        const Quaternion& Rotation,
        const Vector3&    Translation)
    {
        // Composite the frustum rotation and the transform rotation.
        Orientation = Orientation * Rotation;

        // Transform the origin.
        Origin = glm::rotate(Rotation, Origin * Scale) + Translation;

        // Scale the near and far distances (the slopes remain the same).
        Near = Near * Scale;
        Far  = Far * Scale;
    }

    //

    ContainmentType Frustum::Contains(
        const AABB& Box) const
    {
        return Box.Contains(GetPlanes());
    }

    ContainmentType Frustum::Contains(
        const Frustum& Fr) const
    {
        return Fr.Contains(GetPlanes());
    }

    ContainmentType Frustum::Contains(
        std::span<const Math::Plane> Planes) const
    {
        // Set w of the origin to one so we can dot4 with a plane.
        Vector4 OriginW1(Origin, 1.f);

        // Build the corners of the frustum (in world space).
        Vector4 RightTop(RightSlope, TopSlope, 1.0f, 0.0f);
        Vector4 RightBottom(RightSlope, BottomSlope, 1.0f, 0.0f);
        Vector4 LeftTop(LeftSlope, TopSlope, 1.0f, 0.0f);
        Vector4 LeftBottom(LeftSlope, BottomSlope, 1.0f, 0.0f);

        RightTop    = glm::rotate(Orientation, RightTop);
        RightBottom = glm::rotate(Orientation, RightBottom);
        LeftTop     = glm::rotate(Orientation, LeftTop);
        LeftBottom  = glm::rotate(Orientation, LeftBottom);

        std::array Corners{
            RightTop * Near + OriginW1,
            RightBottom * Near + OriginW1,
            LeftTop * Near + OriginW1,
            LeftBottom * Near + OriginW1,
            RightTop * Far + OriginW1,
            RightBottom * Far + OriginW1,
            LeftTop * Far + OriginW1,
            LeftBottom * Far + OriginW1
        };

        bool AnyOutside = false, AllInside = false;
        Planes[0].IntersectFrustum(Corners, AnyOutside, AllInside);

        for (size_t i = 1; i < Planes.size(); i++)
        {
            bool CurAnyOutside, CurAllInside;
            Planes[i].IntersectFrustum(Corners, CurAnyOutside, CurAllInside);

            AnyOutside |= CurAnyOutside;
            AllInside &= CurAllInside;
        }

        return AnyOutside  ? ContainmentType::Disjoint
               : AllInside ? ContainmentType::Contains
                           : ContainmentType::Intersects;
    }
} // namespace Neon::Geometry