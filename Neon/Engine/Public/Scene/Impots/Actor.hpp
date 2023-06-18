#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene::Imports
{
    /// <summary>
    /// Register all math components.
    /// </summary>
    void RegisterActorComponents(
        flecs::world& world);
} // namespace Neon::Scene::Imports