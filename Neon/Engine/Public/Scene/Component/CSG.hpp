#pragma once

#include <Scene/Component/Component.hpp>
#include <Scene/CSG/Shape.hpp>

namespace Neon::Scene::Component
{
    struct CSGBrush
    {
        NEON_COMPONENT_SERIALIZE_IMPL
        {
        }

        NEON_EXPORT_FLECS_COMPONENT(CSGBrush, "CSGBrush")
        {
            NEON_COMPONENT_SERIALIZE(CSGBrush);
        }

        CSG::Brush Brush;
    };
} // namespace Neon::Scene::Component