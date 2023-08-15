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
        World.component<Vector2>("Vector2");
        World.component<Vector2I>("Vector2I");
        World.component<Vector2U>(":Vector2U");

        World.component<Vector3>("Vector3");
        World.component<Vector3I>("Vector3I");
        World.component<Vector3U>("Vector3U");

        World.component<Vector4>("Vector4");
        World.component<Vector4I>("Vector4I");
        World.component<Vector4U>("Vector4U");

        World.component<Quaternion>("Quaternion");

        //

        World.component<Color3U8>("Color3U8");
        World.component<Color4U8>("Color4U8");

        //

        World.component<Matrix3x3>("Matrix3x3");
        World.component<Matrix4x4>("Matrix4x4");

        //

        World.component<RectI>("RectI");
        World.component<RectF>("RectF");

        //

        World.component<TransformMatrix>("TransformMatrix");
        World.component<AffineTransformMatrix>("AffineTransformMatrix");
    }
} // namespace Neon::Scene::Exports