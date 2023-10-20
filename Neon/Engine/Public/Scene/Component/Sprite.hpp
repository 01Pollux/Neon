#pragma once

#include <Scene/Component/Component.hpp>
#include <RHI/Material/Material.hpp>
#include <Math/Transform.hpp>

namespace Neon::Scene::Component
{
    struct Sprite
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
            Archive& TextureTransform& SpriteSize& ModulationColor;
        }

        NEON_EXPORT_FLECS_COMPONENT(Sprite, "Sprite")
        {
            NEON_COMPONENT_SERIALIZE(Sprite);
        }

        /// <summary>
        /// Tag component for the custom renderer of a sprite.
        /// Used for query in gbuffer rendering pass for sprites.
        /// </summary>
        struct CustomRenderer
        {
            NEON_EXPORT_FLECS(CustomRenderer, "Sprite_CustomRenderer")
            {
            }
        };

        Sprite();

        Ptr<RHI::IMaterial> MaterialInstance;
        TransformMatrix     TextureTransform;
        Vector2             SpriteSize{ 1.0f, 1.0f };
        Color4              ModulationColor = Colors::White;
    };
} // namespace Neon::Scene::Component