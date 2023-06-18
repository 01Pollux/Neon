#pragma once

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>

namespace Neon::Scene::Component
{
    struct CanvasItem
    {
        Color4 ModulationColor = Colors::White;
        RectI  TextureRect;
    };
} // namespace Neon::Scene::Component