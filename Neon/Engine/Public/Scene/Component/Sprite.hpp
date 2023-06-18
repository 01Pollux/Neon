#pragma once

#include <Scene/Component/CanvasItem.hpp>

namespace Neon::Scene::Component
{
    struct Sprite : CanvasItem
    {
        static void Register(
            flecs::world& World)
        {
            CanvasItem::Register(World);
        }
    };

    struct Sprite2 : CanvasItem
    {
        static void Register(
            flecs::world& World)
        {
            CanvasItem::Register(World);
        }
    };
} // namespace Neon::Scene::Component