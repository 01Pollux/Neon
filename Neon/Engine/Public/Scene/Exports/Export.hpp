#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene::Exports
{
    /// <summary>
    /// Register all components for flecs.
    /// </summary>
    void RegisterComponents(
        flecs::world& World);
} // namespace Neon::Scene::Exports