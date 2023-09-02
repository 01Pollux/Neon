#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene
{
    struct EntityHandle
    {
    public:
        EntityHandle(
            flecs::entity_t Entity = {}) noexcept :
            m_Entity(Entity)
        {
        }
        EntityHandle(
            flecs::entity Entity) noexcept :
            m_Entity(Entity)
        {
        }

        /// <summary>
        /// Create a new entity with unique name if needed.
        /// </summary>
        static EntityHandle Create(
            EntityHandle SceneHandle,
            const char*  Name = nullptr);

        /// <summary>
        /// Create a new entity with unique name if needed.
        /// </summary>
        static EntityHandle Create(
            EntityHandle SceneHandle,
            EntityHandle ParentHandle,
            const char*  Name = nullptr);

        /// <summary>
        /// Get the entity.
        /// </summary>
        [[nodiscard]] flecs::entity Get() const noexcept;

        /// <summary>
        /// Get the entity id.
        /// </summary>
        [[nodiscard]] flecs::entity_t GetId() const noexcept
        {
            return m_Entity;
        }

    public:
        /// <summary>
        /// Get scene tag.
        /// </summary>
        [[nodiscard]] EntityHandle GetSceneTag() const;

        /// <summary>
        /// Ssafely set the entity name.
        /// </summary>
        void SetName(
            const char* Name);

    public:
        /// <summary>
        /// Delete the entity.
        /// </summary>
        void Delete(
            bool WithChildren = false);

        /// <summary>
        /// Clones the entity into the same parent.
        /// </summary>
        EntityHandle CloneToParent(
            EntityHandle SceneHandle,
            const char*  Name = nullptr) const;

        /// <summary>
        /// Clones the entity.
        /// </summary>
        EntityHandle CloneTo(
            EntityHandle    SceneHandle,
            flecs::entity_t NewParent,
            const char*     Name = nullptr) const;

        /// <summary>
        /// Clones the entity.
        /// </summary>
        EntityHandle CloneToRoot(
            EntityHandle SceneHandle,
            const char*  Name = nullptr) const;

    public:
        operator flecs::entity_t() const noexcept
        {
            return Get();
        }

        operator flecs::entity_t() noexcept
        {
            return m_Entity;
        }

        operator flecs::entity() const noexcept
        {
            return Get();
        }

        operator flecs::entity() noexcept
        {
            return Get();
        }

        operator bool() const noexcept
        {
            return m_Entity != 0;
        }

    private:
        flecs::entity_t m_Entity{};
    };

    //

    struct EntityWorld
    {
    public:
        /// <summary>
        /// Create a global entity world.
        /// </summary>
        static void Initialize();

        /// <summary>
        /// Release the global entity world.
        /// </summary>
        static void Shutdown();

        /// <summary>
        /// Get the singleton instance of the entity world.
        /// </summary>
        [[nodiscard]] static flecs::world Get();

    public:
        /// <summary>
        /// Create a filter to get all children of an entity regardless of whether they are disabled or not.
        /// </summary>
        [[nodiscard]] static flecs::filter_builder<> GetChildrenFilter(
            EntityHandle Parent);

        /// <summary>
        /// Create a filter to get all root entities with a scene tag.
        /// </summary>
        [[nodiscard]] static flecs::filter_builder<> GetRootFilter(
            EntityHandle SceneTag);

        /// <summary>
        /// Get the active scene.
        /// </summary>
        [[nodiscard]] static flecs::entity GetCurrentSceneTag();

        /// <summary>
        /// Set the active scene.
        /// </summary>
        [[nodiscard]] static void SetCurrentSceneTag(
            flecs::entity Tag);
    };
} // namespace Neon::Scene