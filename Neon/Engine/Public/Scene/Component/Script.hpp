#pragma once

#include <Scene/Component/Component.hpp>
#include <Script/Handle.hpp>

namespace Neon::Runtime
{
    class GameLogic;
} // namespace Neon::Runtime

namespace Neon::Scene::Component
{
    struct ScriptInstance
    {
        friend class Runtime::GameLogic;

        NEON_COMPONENT_SERIALIZE_IMPL
        {
            Archive& AssemblyName& ClassName;
        }

        NEON_EXPORT_FLECS_COMPONENT(ScriptInstance, "ScriptInstance")
        {
            NEON_COMPONENT_SERIALIZE(ScriptInstance);
        }

        StringU8 AssemblyName;
        StringU8 ClassName;

    private:
        Scripting::GCHandle Handle;
    };
} // namespace Neon::Scene::Component