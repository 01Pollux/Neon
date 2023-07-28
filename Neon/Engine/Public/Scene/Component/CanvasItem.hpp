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
        RectI  TextureRect     = RectI(Vec::Zero<Vector2I>, Vec::One<Vector2I>);
        Size2  Size            = Size2(1.0f, 1.0f);
    };
} // namespace Neon::Scene::Component