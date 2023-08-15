#pragma once

#include <Scene/Component/Camera.hpp>

namespace Neon::RG
{
    class RenderGraph;

    /// <summary>
    /// Create a standard 2D render graph.
    /// This render graph is used for 2D rendering.
    ///
    /// This render graph contains the following passes:
    /// - GBufferPass
    /// - DebugPass
    /// </summary>
    [[nodiscard]] void CreateStandard2DRenderGraph(
        Scene::Component::Camera& CameraComponent,
        const flecs::entity&      Camera);
} // namespace Neon::RG