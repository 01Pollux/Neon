#pragma once

#include <Scene/Component/Component.hpp>

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>

namespace Neon::Scene::Component
{
    struct CanvasItem
    {
        NEON_EXPORT_COMPONENT();

        Color4 ModulationColor = Colors::White;
        RectI  TextureRect;
    };
} // namespace Neon::Scene::Component