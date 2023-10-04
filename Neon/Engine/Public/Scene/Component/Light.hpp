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

        /// <summary>
        /// Color modulation
        /// </summary>
        Color4 Color = Colors::White;
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

        /// <summary>
        /// Direction of light.
        /// </summary>
        Vector3 Direction = Vec::Zero<Vector3>;
    };

    struct PointLight : Light
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
        }

        NEON_EXPORT_FLECS_COMPONENT(PointLight, "PointLight")
        {
            Component.is_a<Light>();
            NEON_COMPONENT_SERIALIZE(PointLight);
        }

        /// <summary>
        /// Max range of light.
        /// </summary>
        float Range = 5.f;

        /// <summary>
        /// Attenuation of the light.
        /// </summary>
        float Attenuation = 1.f;
    };

    struct SpotLight : Light
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
        }

        NEON_EXPORT_FLECS_COMPONENT(SpotLight, "SpotLight")
        {
            Component.is_a<Light>();
            NEON_COMPONENT_SERIALIZE(SpotLight);
        }

        /// <summary>
        /// Max range of light.
        /// </summary>
        float Range = 5.f;

        /// <summary>
        /// Attenuation of the light.
        /// </summary>
        float Attenuation = 1.f;

        /// <summary>
        /// Angle of the cone.
        /// </summary>
        float Angle = 45.f;

        /// <summary>
        /// Attenuation of the cone.
        /// </summary>
        float AngleAttenuation = 1.f;
    };
} // namespace Neon::Scene::Component