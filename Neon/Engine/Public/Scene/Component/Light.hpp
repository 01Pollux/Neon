#pragma once

#include <Scene/Component/Component.hpp>
#include <Mdl/Mesh.hpp>

namespace Neon::Scene::Component
{
    struct Light
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
        }

        NEON_EXPORT_FLECS_COMPONENT(Light, "Light")
        {
            NEON_COMPONENT_SERIALIZE(Light);
        }
    };

    struct DirectionalLight : Light
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
        }

        NEON_EXPORT_FLECS_COMPONENT(DirectionalLight, "DirectionalLight")
        {
            Component.is_a<Light>();
            NEON_COMPONENT_SERIALIZE(DirectionalLight);
        }
    };
} // namespace Neon::Scene::Component