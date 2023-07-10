#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene::Exports
{
    /// <summary>
    /// Register all actor components.
    /// </summary>
    void RegisterActorComponents(
        flecs::world& world);
} // namespace Neon::Scene::Exports