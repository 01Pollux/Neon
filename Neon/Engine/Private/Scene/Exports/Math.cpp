#include <EnginePCH.hpp>
#include <Scene/Exports/Export.hpp>

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>
#include <Math/Matrix.hpp>
#include <Math/Transform.hpp>

namespace Neon::Scene::Exports
{
    void RegisterMathComponents(flecs::world& World)
    {
        World.component<Vector2>("Neon::Vector2");
        World.component<Vector2I>("Neon::Vector2I");
        World.component<Vector2U>("Neon::Vector2U");

        World.component<Vector3>("Neon::Vector3");
        World.component<Vector3I>("Neon::Vector3I");
        World.component<Vector3U>("Neon::Vector3U");

        World.component<Vector4>("Neon::Vector4");
        World.component<Vector4I>("Neon::Vector4I");
        World.component<Vector4U>("Neon::Vector4U");

        //

        World.component<Color3U8>("Neon::Color3U8");
        World.component<Color4U8>("Neon::Color4U8");

        //

        World.component<Matrix3x3>("Neon::Matrix3x3");
        World.component<Matrix4x4>("Neon::Matrix4x4");

        //

        World.component<RectI>("Neon::RectI");
        World.component<RectF>("Neon::RectF");

        //

        World.component<TransformMatrix>("Neon::Transform");
    }
} // namespace Neon::Scene::Exports