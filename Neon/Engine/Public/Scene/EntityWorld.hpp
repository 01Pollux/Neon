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
            const char* Name);

        /// <summary>
        /// Create a new entity with unique name if needed.
        /// </summary>
        static EntityHandle Create(
            EntityHandle ParentHandle,
            const char*  Name);

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
        /// Delete the entity.
        /// </summary>
        void Delete(
            bool WithChildren = false);

        /// <summary>
        /// Clones the entity into the same parent.
        /// </summary>
        void Clone(
            const char* Name = nullptr);

        /// <summary>
        /// Clones the entity.
        /// </summary>
        EntityHandle Clone(
            flecs::entity_t NewParent,
            const char*     Name = nullptr);

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

        /// <summary>
        /// Get the root entity.
        /// </summary>
        [[nodiscard]] static EntityHandle GetRootEntity();
    };
} // namespace Neon::Scene