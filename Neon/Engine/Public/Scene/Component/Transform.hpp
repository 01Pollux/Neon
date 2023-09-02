#pragma once

#include <Scene/Component/Component.hpp>
#include <Math/Transform.hpp>

namespace Neon::Scene::Component
{
    struct Transform
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
            Archive& World;
        }

        NEON_EXPORT_FLECS_COMPONENT(Transform, "Transform")
        {
            NEON_COMPONENT_SERIALIZE(Transform);
        }

        TransformMatrix World;
    };
} // namespace Neon::Scene::Component