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
    //

    enum class EntityType
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
        /// Get the flecs world.
        /// </summary>
        [[nodiscard]] auto operator->() const noexcept
        {
            return m_World.get();
        }

        /// <summary>
        /// Get the flecs world.
        /// </summary>
        [[nodiscard]] auto Get() const noexcept
        {
            return m_World.get();
        }

        /// <summary>
        /// Render the scene to all active cameras.
        /// </summary>
        void Render();

    public:
        /// <summary>
        /// Create a new entity.
        /// </summary>
        Actor CreateEntity(
            EntityType  Type,
            const char* Name = nullptr);

    private:
        UPtr<flecs::world> m_World;

        flecs::query<
            Component::Transform,
            Component::Camera>
            m_CameraQuery;
    };
} // namespace Neon::Scene