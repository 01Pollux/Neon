#pragma once

#include <Renderer/RG/RG.hpp>
#include <Scene/Scene.hpp>

namespace Neon::RG
{
    class Standard2DRenderGraph : public RG::RenderGraph
    {
    public:
        Standard2DRenderGraph(
            Scene::GameScene& Scene);
    };
} // namespace Neon::RG