#pragma once

#include <Core/Neon.hpp>
#include <Scene/Component/Component.hpp>

namespace Neon
{
    namespace Scene::Component
    {
        struct Camera;
    } // namespace Scene::Component

    namespace Physics
    {
        class World;
    } // namespace Physics
} // namespace Neon

namespace Neon::Runtime
{
    class GameLogic
    {
    public:
        GameLogic();

        NEON_CLASS_NO_COPY(GameLogic);
        NEON_CLASS_MOVE(GameLogic);

        ~GameLogic();

    public:
        /// <summary>
        /// Get the singleton instance of the engine's logic.
        /// </summary>
        static GameLogic* Get();

        /// <summary>
        /// Render the scene to all active cameras.
        /// </summary>
        void Render();

        /// <summary>
        /// Update the scene physics.
        /// </summary>
        void Update();

    public:
        /// <summary>
        /// Get the physics world.
        /// </summary>
        [[nodiscard]] Physics::World* GetPhysicsWorld() const
        {
            return m_PhysicsWorld.get();
        }

        /// <summary>
        /// Get the entity world.
        /// </summary>
        [[nodiscard]] flecs::world* GetEntityWorld() const
        {
            return m_EntityWorld.get();
        }

    public:
        /// <summary>
        /// Create a new entity.
        /// </summary>
        [[nodiscard]] flecs::entity CreateEntity(
            const char* Name = nullptr);

        /// <summary>
        /// Create a new entity in the root world.
        /// The world's root must be set.
        /// </summary>
        [[nodiscard]] flecs::entity CreateEntityInRoot(
            const char* Name = nullptr);

        /// <summary>
        /// Create a new entity.
        /// </summary>
        [[nodiscard]] flecs::entity CreateRootEntity(
            const char* Name = nullptr);

        /// <summary>
        /// Get the root entity.
        /// </summary>
        [[nodiscard]] flecs::entity GetRootEntity() const;

        /// <summary>
        /// Set the root entity.
        /// </summary>
        [[nodiscard]] void SetRootEntity(
            const flecs::entity& Entity);

    private:
        UPtr<Physics::World> m_PhysicsWorld;
        UPtr<flecs::world>   m_EntityWorld;

        flecs::query<
            Scene::Component::Camera>
            m_CameraQuery;
    };
} // namespace Neon::Runtime