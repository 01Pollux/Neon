#pragma once

#include <flecs/flecs.h>

namespace Neon::Scene
{
    struct EntityWorld : public flecs::world
    {
    public:
        using flecs::world::world;

        EntityWorld(
            flecs::world&& World) :
            flecs::world(std::move(World))
        {
        }

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
    };
} // namespace Neon::Scene