#pragma once

#include <Core/String.hpp>
#include <flecs/flecs.h>

#define NEON_EXPORT_FLECS(Class, Name)                  \
    static void RegisterFlecs(                          \
        flecs::world& World)                            \
    {                                                   \
        _HandleComponent(World.component<Class>(Name)); \
    }                                                   \
    static void _HandleComponent(                       \
        flecs::entity Component)

#define NEON_REGISTER_FLECS(ClassName) \
    ClassName::RegisterFlecs(World);

namespace Neon::Scene::Component
{
    /// <summary>
    /// Tag component for entities that are part of the scene and should be rendered, updated, etc.
    /// </summary>
    struct SceneEntity
    {
        NEON_EXPORT_FLECS(SceneEntity, "SceneEntity")
        {
        }
    };
} // namespace Neon::Scene::Component
