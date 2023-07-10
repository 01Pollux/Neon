#include <EnginePCH.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Transform.hpp>
#include <Renderer/RG/RG.hpp>

namespace Neon::Scene::Component
{
    Camera::Camera() :
        RenderGraph(std::make_unique<RG::RenderGraph>())
    {
    }

    float Camera::Viewport::AspectRatio() const
    {
        return Width / Height;
    }

    Matrix4x4 Camera::Viewport::ProjectionMatrix(
        CameraType Type) const
    {
        switch (Type)
        {
        case CameraType::Perspective:
            return glm::perspectiveFov(
                FieldOfView,
                Width,
                Height,
                NearPlane,
                FarPlane);
        case CameraType::Orthographic:
            return glm::ortho(
                -Width / 2.0f,
                Width / 2.0f,
                -Height / 2.0f,
                Height / 2.0f,
                NearPlane,
                FarPlane);
        default:
            return Mat::Identity<Matrix4x4>;
        }
    }

    Matrix4x4 Camera::ProjectionMatrix() const
    {
        return Viewport.ProjectionMatrix(Type);
    }

    constexpr Vector3 p()
    {
        Vector3 x{};
        x.y = 1;
        return x;
    };

    Matrix4x4 Camera::ViewMatrix(
        const flecs::entity& OwningEntity)
    {
        auto TransformComponent = OwningEntity.get<Component::Transform>();
        return glm::lookAt(
            TransformComponent->World.GetPosition(),
            LookAt,
            Vec::Up<Vector3>);
    }

    Matrix4x4 Camera::ViewProjectionMatrix(
        const flecs::entity& OwningEntity)
    {
        return ViewMatrix(OwningEntity) * ProjectionMatrix();
    }
} // namespace Neon::Scene::Component