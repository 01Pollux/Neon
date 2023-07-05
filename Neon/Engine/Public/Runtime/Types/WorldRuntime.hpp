#pragma once

#include <Runtime/Runtime.hpp>
#include <Scene/Scene.hpp>

namespace Neon::Asset
{
    class ShaderLibraryAsset;
}

namespace Neon::Runtime
{
    class EngineRenderer;

    class EngineWorldRuntime : public EngineRuntime
    {
    public:
        EngineWorldRuntime(
            DefaultGameEngine* Engine);

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
        void SetupRenderPasses(
            EngineRenderer*                       Renderer,
            const Ptr<Asset::ShaderLibraryAsset>& ShaderLibrary);

    private:
        Scene::GameScene m_Scene;
    };
} // namespace Neon::Runtime