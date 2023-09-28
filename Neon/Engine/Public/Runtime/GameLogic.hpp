#pragma once

#include <Core/Neon.hpp>
#include <Scene/EntityWorld.hpp>
#include <Scene/GPUScene.hpp>
#include <RHI/Commands/Context.hpp>

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
        /// Get the GPU scene.
        /// </summary>
        [[nodiscard]] Scene::GPUScene* GetGPUScene() 
        {
            return &m_GpuScene;
        }

    private:
        UPtr<Physics::World> m_PhysicsWorld;

        Scene::GPUScene m_GpuScene;

        flecs::query<
            Scene::Component::Camera>
            m_CameraQuery;
    };
} // namespace Neon::Runtime