#pragma once

#include <Core/String.hpp>
#include <flecs/flecs.h>
#include <functional>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

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

#define NEON_EXPORT_FLECS_ENUM(Enum, Name)           \
    static void Enum##_RegisterFlecs()               \
    {                                                \
        Enum##_HandleComponent(                      \
            Neon::Scene::Component::Impl::GetWorld() \
                .component<Enum>(Name));             \
    }                                                \
    static void Enum##_HandleComponent(              \
        flecs::entity Component)

#if NEON_EDITOR
#define NEON_EXPORT_FLECS_COMPONENT(Class, Name)                                             \
    static void RegisterFlecs()                                                              \
    {                                                                                        \
        extern void Insecptor_Component_On##Class(                                           \
            flecs::entity_t, flecs::id_t);                                                   \
        _HandleComponent(                                                                    \
            Neon::Scene::Component::Impl::GetWorld()                                         \
                .component<Class>(Name)                                                      \
                .emplace<Neon::Scene::Component::EditorMetaData>(                            \
                    &Insecptor_Component_On##Class,                                          \
                    [](flecs::entity_t Ent, flecs::id_t Id)                                  \
                    {                                                                        \
                        flecs::entity Entity(Neon::Scene::Component::Impl::GetWorld(), Ent); \
                        Entity.set<Class>({});                                               \
                    }));                                                                     \
    }                                                                                        \
    static void _HandleComponent(                                                            \
        flecs::entity Component)
#else
#define NEON_EXPORT_FLECS_COMPONENT NEON_EXPORT_FLECS
#endif

//

#define NEON_COMPONENT_SERIALIZE_FUNC(Class, SerializeFuncName, DeserializefuncName)      \
    Component.emplace<Neon::Scene::Component::ComponentSerializer>(                       \
        [](boost::archive::text_oarchive& Archive, flecs::entity_t EntityId, flecs::id_t) \
        {                                                                                 \
            flecs::entity Ent(Neon::Scene::Component::Impl::GetWorld(), EntityId);        \
            const_cast<Class*>(Ent.get<Class>())->SerializeFuncName(Archive, Ent);        \
        },                                                                                \
        [](boost::archive::text_iarchive& Archive, flecs::entity_t EntityId, flecs::id_t) \
        {                                                                                 \
            flecs::entity Ent(Neon::Scene::Component::Impl::GetWorld(), EntityId);        \
            Ent.get_mut<Class>()->DeserializefuncName(Archive, Ent);                      \
            Ent.modified<Class>();                                                        \
        })

#define NEON_COMPONENT_SERIALIZE(Class)       NEON_COMPONENT_SERIALIZE_FUNC(Class, Serialize, Serialize)
#define NEON_COMPONENT_SERIALIZE_SPLIT(Class) NEON_COMPONENT_SERIALIZE_FUNC(Class, SerializeTo, SerializeFrom)

#define NEON_COMPONENT_SERIALIZE_IMPL \
    template<typename _Archive>       \
    void Serialize(                   \
        _Archive&     Archive,        \
        flecs::entity Entity)

#define NEON_COMPONENT_SERIALIZE_SPLIT_IMPL \
    void SerializeTo(                       \
        _Archive&     Archive,              \
        flecs::entity Entity)

#define NEON_COMPONENT_DESERIALIZE_SPLIT_IMPL \
    void SerializeFrom(                       \
        _Archive&     Archive,                \
        flecs::entity Entity)

//

#define NEON_REGISTER_FLECS(ClassName) \
    ClassName::RegisterFlecs()

#define NEON_REGISTER_FLECS_ENUM(Enum) \
    Enum##_RegisterFlecs()

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
        /// Callback when the component is added to an entity.
        /// </summary>
        mutable std::move_only_function<void(flecs::entity_t, flecs::id_t)> OnAddInitialize;

        /// <summary>
        /// Wether or not this component should be added to the entity editor.
        /// </summary>
        bool IsSceneComponent = true;
#endif
    };

    /// <summary>
    /// Tag for components for serialisation/deserialisation.
    /// </summary>
    struct ComponentSerializer
    {
        mutable std::move_only_function<void(boost::archive::text_oarchive&, flecs::entity_t, flecs::id_t)> Serialize;
        mutable std::move_only_function<void(boost::archive::text_iarchive&, flecs::entity_t, flecs::id_t)> Deserialize;
    };

    /// <summary>
    /// Pair for entities that are part of a certain scene.
    /// Value is the scene root entity.
    /// </summary>
    struct SceneEntity
    {
        NEON_EXPORT_FLECS(SceneEntity, "SceneEntity")
        {
            Component.add(flecs::Exclusive);
        }
    };

    /// <summary>
    /// Pair for world for identifying active scene.
    /// Value is the scene root entity.
    /// </summary>
    struct WorldSceneRoot
    {
        NEON_EXPORT_FLECS(WorldSceneRoot, "WorldSceneRoot")
        {
            Component.add(flecs::Exclusive);
        }
    };

    /// <summary>
    /// Tag for entities that are part of the certain scene and should be rendered, updated, etc.
    /// </summary>
    struct ActiveSceneEntity
    {
        NEON_EXPORT_FLECS(ActiveSceneEntity, "ActiveSceneEntity")
        {
        }
    };
} // namespace Neon::Scene::Component
