#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene
{
    struct EntityWorld
    {
        static constexpr const char* RootEntityName = "_Root";

    public:
        /// <summary>
        /// Get the singleton instance of the entity world.
        /// </summary>
        [[nodiscard]] static EntityWorld* Get();

        EntityWorld();

        EntityWorld(
            flecs::world&& World) :
            m_World(std::exchange(World.m_world, {})),
            m_RootEntity(World.lookup(RootEntityName)),
            m_Owned(std::exchange(World.m_owned, {}))
        {
        }

        EntityWorld(
            const EntityWorld& Other) :
            m_World(Other.m_World),
            m_RootEntity(Other.m_RootEntity),
            m_Owned(false)
        {
        }

        EntityWorld(
            EntityWorld&& Other) noexcept :
            m_World(std::exchange(Other.m_World, {})),
            m_RootEntity(std::exchange(Other.m_RootEntity, {})),
            m_Owned(std::exchange(Other.m_Owned, {}))
        {
        }

        EntityWorld& operator=(
            const EntityWorld& Other)
        {
            if (this != &Other)
            {
                Release();
                m_World      = Other.m_World;
                m_RootEntity = Other.m_RootEntity;
                m_Owned      = false;
            }
            return *this;
        }

        EntityWorld& operator=(
            EntityWorld&& Other) noexcept
        {
            if (this != &Other)
            {
                Release();
                m_World      = std::exchange(Other.m_World, {});
                m_RootEntity = std::exchange(Other.m_RootEntity, {});
                m_Owned      = std::exchange(Other.m_Owned, {});
            }
            return *this;
        }

        ~EntityWorld()
        {
            Release();
        }

        /// <summary>
        /// Release the world.
        /// </summary>
        void Release();

    public:
        /// <summary>
        /// Get the root entity.
        /// </summary>
        [[nodiscard]] flecs::entity GetRootEntity();

    public:
        /// <summary>
        /// Delete the entity with or without children.
        /// </summary>
        static void DeleteEntity(
            flecs::entity Entity,
            bool          WithChildren = false);

        /// <summary>
        /// Clone the entity.
        /// </summary>
        static flecs::entity CloneEntity(
            flecs::entity Entity,
            const char*   Name = nullptr);

    public:
        operator flecs::world_t*() const noexcept
        {
            return m_World;
        }

        operator flecs::world_t*() noexcept
        {
            return m_World;
        }

        operator flecs::world() const noexcept
        {
            return GetWorld();
        }

        operator flecs::world() noexcept
        {
            return GetWorld();
        }

        /// <summary>
        /// Get the world.
        /// </summary>
        [[nodiscard]] flecs::world GetWorld() const noexcept
        {
            return flecs::world(m_World);
        }

        /// <summary>
        /// Get the world.
        /// </summary>
        [[nodiscard]] flecs::world GetWorld() noexcept
        {
            return flecs::world(m_World);
        }

    private:
        flecs::world_t* m_World;
        flecs::entity_t m_RootEntity;
        bool            m_Owned = false;
    };
} // namespace Neon::Scene