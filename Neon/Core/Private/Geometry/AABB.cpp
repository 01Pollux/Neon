#include <CorePCH.hpp>
#include <Geometry/AABB.hpp>
#include <Geometry/Frustum.hpp>

namespace Neon::Geometry
{
    void AABB::Transform(
        float             Scale,
        const Quaternion& Rotation,
        const Vector3&    Translation)
    {
        Vector3 Min(std::numeric_limits<float>::max()),
            Max(std::numeric_limits<float>::lowest());

        // Compute and transform the corners and find new min/max bounds.
        for (size_t i = 0; i < 8; ++i)
        {
            Vector3 Corner = Extents * s_BoxOffset[i] + Center;
            Corner         = glm::rotate(Rotation, Corner * Scale) + Translation;

            Min = glm::min(Min, Corner);
            Max = glm::max(Max, Corner);
        }

        Center  = (Min + Max) * 0.5f;
        Extents = (Max - Min) * 0.5f;
    }

    ContainmentType AABB::Contains(
        const AABB& Box) const
    {
        Vector3 MinA = Min();
        Vector3 MaxA = Max();

        Vector3 MinB = Box.Min();
        Vector3 MaxB = Box.Max();

        // for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
        bool Disjoint = glm::any(glm::greaterThan(MinA, MaxB) | glm::greaterThan(MinB, MaxA));
        if (Disjoint)
        {
            return ContainmentType::Disjoint;
        }

        // for each i in (x, y, z) if a_min(i) <= b_min(i) and b_max(i) <= a_max(i) then A contains B
        bool Inside = glm::all(glm::lessThanEqual(MinA, MinB) && glm::lessThanEqual(MaxB, MaxA));
        return Inside ? ContainmentType::Contains : ContainmentType::Intersects;
    }

    ContainmentType AABB::Contains(
        const Frustum& Fr) const
    {
        return Fr.Contains(*this);
    }

    ContainmentType AABB::Contains(
        std::span<const Math::Plane> Planes) const
    {
        // Set w of the center to one so we can dot4 with a plane.
        Vector4 CenterW1(Center, 1.f);

        bool AnyOutside = false, AllInside = false;
        Planes[0].IntersectAxisAlignedBox(CenterW1, Extents, AnyOutside, AllInside);

        for (size_t i = 1; i < Planes.size(); i++)
        {
            bool CurAnyOutside, CurAllInside;
            Planes[i].IntersectAxisAlignedBox(CenterW1, Extents, CurAnyOutside, CurAllInside);

            AnyOutside |= CurAnyOutside;
            AllInside &= CurAllInside;
        }

        return AnyOutside  ? ContainmentType::Disjoint
               : AllInside ? ContainmentType::Contains
                           : ContainmentType::Intersects;
    }
} // namespace Neon::Geometry