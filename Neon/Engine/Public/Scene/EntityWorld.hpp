#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene
{
    struct EntityWorld
    {
    public:
        EntityWorld();

        EntityWorld(
            flecs::world&& World) :
            m_World(std::exchange(World.m_world, {})),
            m_Owned(std::exchange(World.m_owned, {}))
        {
        }

        EntityWorld(
            const EntityWorld& Other) :
            m_World(Other.m_World),
            m_Owned(false)
        {
        }

        EntityWorld(
            EntityWorld&& Other) noexcept :
            m_World(std::exchange(Other.m_World, {})),
            m_Owned(std::exchange(Other.m_Owned, {}))
        {
        }

        EntityWorld& operator=(
            const EntityWorld& Other)
        {
            if (this != &Other)
            {
                Release();
                m_World = Other.m_World;
                m_Owned = false;
            }
            return *this;
        }

        EntityWorld& operator=(
            EntityWorld&& Other) noexcept
        {
            if (this != &Other)
            {
                Release();
                m_World = std::exchange(Other.m_World, {});
                m_Owned = std::exchange(Other.m_Owned, {});
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
        /// Create a new entity.
        /// </summary>
        flecs::entity CreateEntity(
            const char* Name = nullptr);

        /// <summary>
        /// Create a new entity in the root world.
        /// The world's root must be set.
        /// </summary>
        flecs::entity CreateEntityInRoot(
            const char* Name = nullptr);

        /// <summary>
        /// Create a new entity.
        /// </summary>
        flecs::entity CreateRootEntity(
            const char* Name = nullptr);

        /// <summary>
        /// Get the root entity.
        /// </summary>
        [[nodiscard]] flecs::entity GetRootEntity();

        /// <summary>
        /// Set the root entity.
        /// </summary>
        void SetRootEntity(
            const flecs::entity& Entity);

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
        bool            m_Owned = false;
    };
} // namespace Neon::Scene