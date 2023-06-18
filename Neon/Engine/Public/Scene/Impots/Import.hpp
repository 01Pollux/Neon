#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene::Imports
{
    /// <summary>
    /// Register all components for flecs.
    /// </summary>
    void Import(
        flecs::world& World);
} // namespace Neon::Scene::Imports