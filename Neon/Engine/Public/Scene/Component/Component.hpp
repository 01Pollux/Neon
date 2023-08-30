#pragma once

#include <Core/String.hpp>
#include <flecs/flecs.h>

namespace Neon::Scene::Component::Impl
{
    /// <summary>
    /// Get the world.
    /// </summary>
    [[nodiscard]] flecs::world GetWorld();
} // namespace Neon::Scene::Component::Impl

#define NEON_EXPORT_FLECS(Class, Name)                                                     \
    static void RegisterFlecs()                                                            \
    {                                                                                      \
        _HandleComponent(Neon::Scene::Component::Impl::GetWorld().component<Class>(Name)); \
    }                                                                                      \
    static void _HandleComponent(                                                          \
        flecs::entity Component)

#define NEON_REGISTER_FLECS(ClassName) \
    ClassName::RegisterFlecs();

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
