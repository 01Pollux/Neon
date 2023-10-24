#pragma once

#include <Scene/Component/Camera.hpp>

namespace Neon::RG
{
    /// <summary>
    /// Create a standard 2D render graph.
    /// This render graph is used for 2D rendering.
    ///
    /// This render graph contains the following passes:
    /// </summary>
    [[nodiscard]] void CreateStandard2DRenderGraph(
        RG::RenderGraph* Rendergraph);

    /// <summary>
    /// Create a standard 3D render graph.
    /// This render graph is used for 3D rendering.
    ///
    /// This render graph contains the following passes:
    /// - DepthPrepass
    /// - LightCullPass
    /// - GeometryPass
    /// - CopyToTexturePass
    /// </summary>
    [[nodiscard]] void CreateStandard3DRenderGraph(
        RG::RenderGraph* Rendergraph);
} // namespace Neon::RG