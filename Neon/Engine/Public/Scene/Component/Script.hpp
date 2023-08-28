#pragma once

#include <Scene/Component/Component.hpp>
#include <Script/Handle.hpp>

namespace Neon::Scene::Component
{
    struct Script
    {
        NEON_EXPORT_FLECS(Script, "Script")
        {
        }

        Script(
            const char*   AssemblyName,
            const char*   ClassName,
            flecs::entity Entity);


    };
} // namespace Neon::Scene::Component