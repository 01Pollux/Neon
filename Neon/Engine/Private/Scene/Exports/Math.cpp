#include <EnginePCH.hpp>
#include <Scene/Exports/Export.hpp>

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>
#include <Math/Matrix.hpp>

namespace Neon::Scene::Exports
{
    void RegisterMathComponents(flecs::world& World)
    {
        World.component<Vector2D>("Neon::Vector2D")
            .member<float>("X")
            .member<float>("Y");

        World.component<Vector2DI>("Neon::Vector2DI")
            .member<int>("X")
            .member<int>("Y");

        World.component<Vector3D>("Neon::Vector3D")
            .member<float>("X")
            .member<float>("Y")
            .member<float>("Z");

        World.component<Vector3DI>("Neon::Vector3DI")
            .member<int>("X")
            .member<int>("Y")
            .member<int>("Z");

        World.component<Vector4D>("Neon::Vector4D")
            .member<float>("X")
            .member<float>("Y")
            .member<float>("Z")
            .member<float>("W");

        World.component<Vector4DI>("Neon::Vector4DI")
            .member<int>("X")
            .member<int>("Y")
            .member<int>("Z")
            .member<int>("W");

        World.component<Color4>("Neon::Color")
            .member<float>("R")
            .member<float>("G")
            .member<float>("B")
            .member<float>("A");

        //

        World.component<Matrix3x3>("Neon::Matrix3x3")
            .member<Vector3D>("M0")
            .member<Vector3D>("M1")
            .member<Vector3D>("M2");

        World.component<Matrix4x4>("Neon::Matrix4x4")
            .member<Vector4D>("M0")
            .member<Vector4D>("M1")
            .member<Vector4D>("M2")
            .member<Vector4D>("M3");

        //

        World.component<RectI>("Neon::RectI")
            .member<Vector2D>("Position")
            .member<Vector2D>("Size");

        World.component<RectF>("Neon::RectF")
            .member<Vector2D>("Position")
            .member<Vector2D>("Size");
    }
} // namespace Neon::Scene::Exports