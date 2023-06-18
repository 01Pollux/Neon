#pragma once

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>

namespace Neon::Scene::Component
{
    struct CanvasItem
    {
        static void Register(
            flecs::world& World)
        {
            World.component<CanvasItem>()
                .member<Color4>("ModulationColor")
                .member<RectI>("TextureRect");
        }

        Color4 ModulationColor = Colors::White;
        RectI  TextureRect;
    };
} // namespace Neon::Scene::Component