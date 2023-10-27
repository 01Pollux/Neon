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

    struct CSGBox3D : public CSGShape, public Scene::CSGBox3D
    {
        using BaseClass = Scene::CSGBox3D;

        using BaseClass::BaseClass;

        CSGBox3D(
            const BaseClass& Shape) :
            BaseClass(Shape)
        {
        }

        NEON_COMPONENT_SERIALIZE_IMPL
        {
            Archive&* this;
        }

        NEON_EXPORT_FLECS_COMPONENT(CSGBox3D, "CSGBox3D")
        {
            NEON_COMPONENT_SERIALIZE(CSGBox3D);
        }
    };
} // namespace Neon::Scene::Component