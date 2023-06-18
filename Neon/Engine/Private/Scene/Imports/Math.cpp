#include <EnginePCH.hpp>
#include <Scene/Impots/Math.hpp>

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>

namespace Neon::Scene::Imports
{
    void RegisterMathComponents(flecs::world& World)
    {
        World.component<Vector2D>("Vector2D")
            .member<float>("X")
            .member<float>("Y");

        World.component<Vector2DI>("Vector2DI")
            .member<int>("X")
            .member<int>("Y");

        World.component<Vector3D>("Vector3D")
            .member<float>("X")
            .member<float>("Y")
            .member<float>("Z");

        World.component<Vector3DI>("Vector3DI")
            .member<int>("X")
            .member<int>("Y")
            .member<int>("Z");

        World.component<Vector4D>("Vector4D")
            .member<float>("X")
            .member<float>("Y")
            .member<float>("Z")
            .member<float>("W");

        World.component<Vector4DI>("Vector4DI")
            .member<int>("X")
            .member<int>("Y")
            .member<int>("Z")
            .member<int>("W");

        World.component<Color4>("Color")
            .member<float>("R")
            .member<float>("G")
            .member<float>("B")
            .member<float>("A");

        World.component<RectI>("RectI")
            .member<Vector2D>("Position")
            .member<Vector2D>("Size");

        World.component<RectF>("RectF")
            .member<Vector2D>("Position")
            .member<Vector2D>("Size");
    }
} // namespace Neon::Scene::Imports