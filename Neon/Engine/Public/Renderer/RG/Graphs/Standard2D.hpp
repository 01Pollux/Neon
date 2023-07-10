#pragma once

#include <Renderer/RG/RG.hpp>

namespace Neon::Scene
{
    class GameScene;
} // namespace Neon::Scene

namespace Neon::RG
{
    /// <summary>
    /// Create a standard 2D render graph.
    /// This render graph is used for 2D rendering.
    ///
    /// This render graph contains the following passes:
    /// - ScenePass
    /// </summary>
    [[nodiscard]] UPtr<RG::RenderGraph> CreateStandard2DRenderGraph(Scene::GameScene& Scene);
} // namespace Neon::RG