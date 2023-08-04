#pragma once

#include <Runtime/Runtime.hpp>
#include <Renderer/RG/Graph.hpp>
#include <Scene/Scene.hpp>

namespace Neon::Runtime
{
    class EngineWorldRuntime : public IEngineRuntime
    {
    public:
        EngineWorldRuntime();

        void Run() override;

        /// <summary>
        /// Get the current scene.
        /// </summary>
        Scene::GameScene& GetScene();

        /// <summary>
        /// Get the current scene.
        /// </summary>
        const Scene::GameScene& GetScene() const;

    private:
        Scene::GameScene m_Scene;
        bool             m_IsRendering = false;
    };
} // namespace Neon::Runtime