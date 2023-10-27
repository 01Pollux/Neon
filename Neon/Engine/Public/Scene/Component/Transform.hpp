#pragma once

#include <Scene/Component/Component.hpp>
#include <Math/Transform.hpp>

namespace Neon::Scene::Component
{
    struct Transform : public TransformMatrix
    {
        using TransformMatrix::TransformMatrix;

        Transform(const TransformMatrix& Mat) :
            TransformMatrix(Mat)
        {
        }

        NEON_COMPONENT_SERIALIZE_IMPL
        {
            Archive& boost::serialization::base_object<TransformMatrix>(*this);
        }

        NEON_EXPORT_FLECS_COMPONENT(Transform, "Transform")
        {
            NEON_COMPONENT_SERIALIZE(Transform);
        }
    };
} // namespace Neon::Scene::Component