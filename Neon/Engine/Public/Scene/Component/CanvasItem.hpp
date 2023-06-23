#pragma once

#include <Scene/Component/Component.hpp>

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>
#include <Math/Size2.hpp>

namespace Neon::Scene::Component
{
    struct CanvasItem
    {
        NEON_EXPORT_COMPONENT();

        Color4 ModulationColor = Colors::White;
        RectI  TextureRect     = RectI(Vector2DI::Zero, Vector2DI::One);
        Size2F Scale           = Size2F(1.0f, 1.0f);
    };
} // namespace Neon::Scene::Component