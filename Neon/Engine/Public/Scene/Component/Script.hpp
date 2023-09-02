#pragma once

#include <Scene/Component/Component.hpp>
#include <Script/Handle.hpp>

namespace Neon::Scene::Component
{
    struct ScriptInstance
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
            Archive& AssemblyName& ClassName;
        }

        NEON_EXPORT_FLECS_COMPONENT(ScriptInstance, "Script Instance")
        {
            NEON_COMPONENT_SERIALIZE(ScriptInstance);
        }

        StringU8 AssemblyName;
        StringU8 ClassName;

        Scripting::GCHandle Handle;
    };
} // namespace Neon::Scene::Component