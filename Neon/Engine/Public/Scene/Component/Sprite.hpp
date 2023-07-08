#pragma once

#include <Scene/Component/CanvasItem.hpp>
#include <Renderer/Material/Material.hpp>

namespace Neon::Scene::Component
{
    struct Sprite : CanvasItem
    {
        NEON_EXPORT_COMPONENT();

        Ptr<Renderer::IMaterialInstance> MaterialInstance;
    };
} // namespace Neon::Scene::Component