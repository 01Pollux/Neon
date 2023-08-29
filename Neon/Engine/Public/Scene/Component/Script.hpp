#pragma once

#include <Scene/Component/Component.hpp>
#include <Script/Handle.hpp>

namespace Neon::Scene::Component
{
    struct ScriptInstance
    {
        NEON_EXPORT_FLECS(ScriptInstance, "ScriptInstance")
        {
        }

        StringU8 AssemblyName;
        StringU8 ClassName;

        Scripting::GCHandle Handle;
    };
} // namespace Neon::Scene::Component