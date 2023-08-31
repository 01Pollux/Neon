#pragma once

#include <Core/String.hpp>
#include <flecs/flecs.h>
#include <functional>

namespace Neon::Scene::Component::Impl
{
    /// <summary>
    /// Get the world.
    /// </summary>
    [[nodiscard]] flecs::world GetWorld();
} // namespace Neon::Scene::Component::Impl

#define NEON_EXPORT_FLECS(Class, Name)               \
    static void RegisterFlecs()                      \
    {                                                \
        _HandleComponent(                            \
            Neon::Scene::Component::Impl::GetWorld() \
                .component<Class>(Name));            \
    }                                                \
    static void _HandleComponent(                    \
        flecs::entity Component)

#if NEON_EDITOR
#define NEON_EXPORT_FLECS_COMPONENT(Class, Name)                  \
    static void RegisterFlecs()                                   \
    {                                                             \
        extern void Insecptor_Component_On##Class(                \
            flecs::entity_t, flecs::id_t);                        \
        _HandleComponent(                                         \
            Neon::Scene::Component::Impl::GetWorld()              \
                .component<Class>(Name)                           \
                .emplace<Neon::Scene::Component::EditorMetaData>( \
                    &Insecptor_Component_On##Class));             \
    }                                                             \
    static void _HandleComponent(                                 \
        flecs::entity Component)
#else
#define NEON_EXPORT_FLECS_COMPONENT NEON_EXPORT_FLECS
#endif

#define NEON_REGISTER_FLECS(ClassName) \
    ClassName::RegisterFlecs();

namespace Neon::Scene::Component
{
    /// <summary>
    /// Tag for components that can be added to entities. (Used for editor)
    /// </summary>
    struct EditorMetaData
    {
        NEON_EXPORT_FLECS(EditorMetaData, "EditorMetaData")
        {
        }

#if NEON_EDITOR
        /// <summary>
        /// Callback for when the component is being inspected.
        /// </summary>
        mutable std::move_only_function<void(flecs::entity_t, flecs::id_t)> RenderOnInsecptorCallback;

        /// <summary>
        /// Wether or not this component should be added to the entity editor.
        /// </summary>
        bool IsSceneComponent = true;
#endif
    };

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
