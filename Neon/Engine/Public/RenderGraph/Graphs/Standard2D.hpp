#pragma once

#include <RenderGraph/RG.hpp>
#include <Scene/Scene.hpp>

namespace Neon::RG
{
    /// <summary>
    /// Create a standard 2D render graph.
    /// This render graph is used for 2D rendering.
    ///
    /// This render graph contains the following passes:
    /// - ScenePass
    /// </summary>
    [[nodiscard]] UPtr<RG::RenderGraph> CreateStandard2DRenderGraph(
        flecs::entity Camera);
} // namespace Neon::RG