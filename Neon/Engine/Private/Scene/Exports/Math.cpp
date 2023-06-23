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
        World.component<Vector2>("Neon::Vector2")
            .member<float>("X")
            .member<float>("Y");

        World.component<Vector2I>("Neon::Vector2I")
            .member<int>("X")
            .member<int>("Y");

        World.component<Vector2U>("Neon::Vector2U")
            .member<uint32_t>("X")
            .member<uint32_t>("Y");

        World.component<Vector3>("Neon::Vector3")
            .member<float>("X")
            .member<float>("Y")
            .member<float>("Z");

        World.component<Vector3I>("Neon::Vector3I")
            .member<int>("X")
            .member<int>("Y")
            .member<int>("Z");

        World.component<Vector3U>("Neon::Vector3U")
            .member<uint32_t>("X")
            .member<uint32_t>("Y")
            .member<uint32_t>("Z");

        World.component<Vector4>("Neon::Vector4")
            .member<float>("X")
            .member<float>("Y")
            .member<float>("Z")
            .member<float>("W");

        World.component<Vector4I>("Neon::Vector4I")
            .member<int>("X")
            .member<int>("Y")
            .member<int>("Z")
            .member<int>("W");

        World.component<Vector4U>("Neon::Vector4U")
            .member<uint32_t>("X")
            .member<uint32_t>("Y")
            .member<uint32_t>("Z")
            .member<uint32_t>("W");

        //

        World.component<Color3U8>("Neon::Color3U8")
            .member<uint8_t>("R")
            .member<uint8_t>("G")
            .member<uint8_t>("B");

        World.component<Color4U8>("Neon::Color4U8")
            .member<uint8_t>("R")
            .member<uint8_t>("G")
            .member<uint8_t>("B")
            .member<uint8_t>("A");

        //

        World.component<Matrix3x3>("Neon::Matrix3x3")
            .member<Vector3>("M0")
            .member<Vector3>("M1")
            .member<Vector3>("M2");

        World.component<Matrix4x4>("Neon::Matrix4x4")
            .member<Vector4>("M0")
            .member<Vector4>("M1")
            .member<Vector4>("M2")
            .member<Vector4>("M3");

        //

        World.component<RectI>("Neon::RectI")
            .member<Vector2>("Position")
            .member<Vector2>("Size");

        World.component<RectF>("Neon::RectF")
            .member<Vector2>("Position")
            .member<Vector2>("Size");

        //

        World.component<TransformMatrix>("Neon::Transform")
            .member<Matrix3x3>("Basis")
            .member<Vector3>("Origin");
    }
} // namespace Neon::Scene::Exports