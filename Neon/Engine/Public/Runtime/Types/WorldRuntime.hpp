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

    private:
        Scene::GameScene m_Scene;
    };
} // namespace Neon::Runtime