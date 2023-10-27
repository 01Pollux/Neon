#pragma once

#include <Scene/Component/Component.hpp>
#include <Scene/CSG/Shape.hpp>

namespace Neon::Scene::Component
{
    /// <summary>
    /// Tag component for CSG shapes.
    /// </summary>
    class CSGShape
    {
    };

    struct CSGBox3D : public CSG::Box3D, public CSGShape
    {
        using CSG::Box3D::Box3D;

        CSGBox3D() = default;

        CSGBox3D(
            const CSG::Box3D& Shape) :
            CSG::Box3D(Shape)
        {
        }

        NEON_COMPONENT_SERIALIZE_IMPL
        {
            Archive& static_cast<CSG::Box3D&>(*this);
        }

        NEON_EXPORT_FLECS_COMPONENT(CSGBox3D, "CSGBox3D")
        {
            Component.is_a<CSGShape>();
            NEON_COMPONENT_SERIALIZE(CSGBox3D);
        }
    };
} // namespace Neon::Scene::Component