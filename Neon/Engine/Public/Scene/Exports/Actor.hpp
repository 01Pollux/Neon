#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene::Exports
{
    /// <summary>
    /// Register all math components.
    /// </summary>
    void RegisterActorComponents(
        flecs::world& world);
} // namespace Neon::Scene::Exports