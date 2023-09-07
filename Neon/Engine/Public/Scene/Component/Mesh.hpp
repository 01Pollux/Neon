#pragma once

#include <Scene/Component/Component.hpp>
#include <Mdl/Mesh.hpp>

namespace Neon::Scene::Component
{
    struct MeshInstance
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
        }

        NEON_EXPORT_FLECS_COMPONENT(MeshInstance, "MeshInstance")
        {
            NEON_COMPONENT_SERIALIZE(MeshInstance);
        }

        Mdl::Mesh Mesh;
    };
} // namespace Neon::Scene::Component