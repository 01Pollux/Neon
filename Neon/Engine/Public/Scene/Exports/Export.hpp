#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene::Exports
{
    /// <summary>
    /// Register all components for flecs.
    /// </summary>
    void RegisterComponents(
        flecs::world& World);

    /// <summary>
    /// Register all relations for flecs.
    /// </summary>
    void RegisterRelations(
        flecs::world& World);
} // namespace Neon::Scene::Exports