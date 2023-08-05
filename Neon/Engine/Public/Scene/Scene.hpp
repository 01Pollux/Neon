#pragma once

#include <Scene/Component/Component.hpp>
#include <Core/Neon.hpp>
#include <flecs/flecs.h>

namespace Neon
{
    namespace Scene::Component
    {
        struct Transform;
        struct Camera;
    } // namespace Scene::Component

    namespace Physics
    {
        class World;
    } // namespace Physics
} // namespace Neon

namespace Neon::Scene
{
    enum class EntityType : uint8_t
    {
        Empty,

        Camera2D,
        Camera3D,

        Sprite,
    };

    //

    class GameScene
    {
    public:
        GameScene();

        NEON_CLASS_NO_COPY(GameScene);
        NEON_CLASS_MOVE(GameScene);

        ~GameScene();

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
        Actor CreateEntity(
            EntityType  Type,
            const char* Name = nullptr);

    private:
        UPtr<Physics::World> m_PhysicsWorld;
        UPtr<flecs::world>   m_EntityWorld;

        flecs::query<
            Component::Transform,
            Component::Camera>
            m_CameraQuery;
    };
} // namespace Neon::Scene