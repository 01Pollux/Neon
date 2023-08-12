#pragma once

#include <Scene/Component/Component.hpp>
#include <Math/Transform.hpp>

namespace Neon::Scene::Component
{
    struct Transform
    {
        NEON_EXPORT_COMPONENT();

        TransformMatrix World;
    };
} // namespace Neon::Scene::Component