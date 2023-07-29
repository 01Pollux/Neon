#pragma once

#include <Scene/Component/Component.hpp>
#include <Core/Neon.hpp>
#include <flecs/flecs.h>

namespace Neon::Scene::Component
{
    struct Transform;
    struct Camera;
} // namespace Neon::Scene::Component

namespace Neon::Scene
{
    class PhysicsWorld;

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
        /// Create a new entity.
        /// </summary>
        Actor CreateEntity(
            EntityType  Type,
            const char* Name = nullptr);

    private:
        UPtr<PhysicsWorld> m_PhysicsWorld;
        UPtr<flecs::world> m_EntityWorld;

        flecs::query<
            Component::Transform,
            Component::Camera>
            m_CameraQuery;
    };
} // namespace Neon::Scene