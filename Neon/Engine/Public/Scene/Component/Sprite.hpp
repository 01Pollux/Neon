#pragma once

#include <Renderer/Material/Material.hpp>
#include <Math/Transform.hpp>

namespace Neon::Scene::Component
{
    struct Sprite
    {
        /// <summary>
        /// Tag component for the main renderer of a sprite.
        /// Used for query in gbuffer rendering pass for sprites.
        /// </summary>
        struct MainRenderer
        {
            NEON_EXPORT_COMPONENT();
        };

        NEON_EXPORT_COMPONENT();

        Ptr<Renderer::IMaterial> MaterialInstance = nullptr;
        AffineTransformMatrix    TextureTransform;
        Vector2                  SpriteSize;
        Color4                   ModulationColor = Colors::White;
    };
} // namespace Neon::Scene::Component