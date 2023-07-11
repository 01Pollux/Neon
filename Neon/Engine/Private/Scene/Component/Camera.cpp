#include <EnginePCH.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Transform.hpp>
#include <Renderer/RG/RG.hpp>

namespace Neon::Scene::Component
{
    Camera::Camera() = default;

    Camera::Camera(
        UPtr<RG::RenderGraph> RenderGraph,
        CameraType            Type) :
        RenderGraph(std::move(RenderGraph)),
        Type(Type)
    {
        if (Type == CameraType::Orthographic)
        {
            Viewport.FieldOfView = 90.0f;
            Viewport.NearPlane   = -1.f;
            Viewport.FarPlane    = 1.f;
        }
    }

    NEON_CLASS_MOVE_IMPL(Camera);

    Camera::~Camera() = default;

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

    Matrix4x4 Camera::ViewMatrix(
        const flecs::entity& OwningEntity) const
    {
        auto TransformComponent = OwningEntity.get<Component::Transform>();
        return glm::lookAt(
            TransformComponent->World.GetPosition(),
            LookAt,
            Vec::Up<Vector3>);
    }

    Matrix4x4 Camera::ViewProjectionMatrix(
        const flecs::entity& OwningEntity) const
    {
        return ViewMatrix(OwningEntity) * ProjectionMatrix();
    }
} // namespace Neon::Scene::Component