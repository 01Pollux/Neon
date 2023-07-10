#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>
#include <flecs/flecs.h>
#include <memory>

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

    using Actor = flecs::entity;

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
        std::unique_ptr<flecs::world> m_World;

        flecs::query<
            Component::Transform,
            Component::Camera>
            m_CameraQuery;
    };
} // namespace Neon::Scene