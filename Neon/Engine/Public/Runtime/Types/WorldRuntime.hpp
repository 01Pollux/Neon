#pragma once

#include <Runtime/Runtime.hpp>
#include <Renderer/RG/Graph.hpp>
#include <Scene/Scene.hpp>

namespace Neon::Asset
{
    class ShaderLibraryAsset;
} // namespace Neon::Asset

namespace Neon::Runtime
{
    class EngineWorldRuntime : public IEngineRuntime
    {
    public:
        EngineWorldRuntime();

        /// <summary>
        /// Get the current scene.
        /// </summary>
        Scene::GameScene& GetScene();

        /// <summary>
        /// Get the current scene.
        /// </summary>
        const Scene::GameScene& GetScene() const;

    public:
        /// <summary>
        /// Import the render graph.
        /// </summary>
        void SetupRenderPasses();

    private:
        Scene::GameScene m_Scene;
        RG::RenderGraph  m_RenderGraph;
        bool             m_WindowIsVisible = false;
    };
} // namespace Neon::Runtime