#pragma once

#include <Scene/Component/Component.hpp>
#include <Scene/GameTimer.hpp>
#include <Core/Neon.hpp>
#include <flecs/flecs.h>

namespace Neon::Scene::Component
{
    struct Transform;
    struct Camera;
} // namespace Neon::Scene::Component

namespace Neon::Physics
{
    class World;
} // namespace Neon::Physics

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
        /// Get total elapsed time for the current level
        /// </summary>
        [[nodiscard]] double GetGameTime() const;

        /// <summary>
        /// Get total elapsed time for the current engine
        /// </summary>
        [[nodiscard]] double GetEngineTime() const;

        /// <summary>
        /// Get delta time
        /// </summary>
        [[nodiscard]] double GetDeltaTime() const;

        /// <summary>
        /// Get time scale
        /// </summary>
        [[nodiscard]] float GetTimeScale() const;

        /// <summary>
        /// Set time scale
        /// </summary>
        void SetTimeScale(
            float TimeScale);

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

        GameTimer m_GameTimer;

        flecs::query<
            Component::Transform,
            Component::Camera>
            m_CameraQuery;
    };
} // namespace Neon::Scene