#include <EnginePCH.hpp>
#include <Scene/Component/Camera.hpp>
#include <Scene/Component/Transform.hpp>

#include <RenderGraph/RG.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::Scene::Component
{
    Camera::Camera(
        CameraType Type) :
        RenderGraph(NEON_NEW RG::RenderGraph),
        Type(Type)
    {
        if (Type == CameraType::Orthographic)
        {
            Viewport.FieldOfView = 90.0f;
            Viewport.NearPlane   = -1.f;
            Viewport.FarPlane    = 1.f;
        }
    }

    Camera::Camera(
        const Camera& Other)
    {
        RenderGraph.reset(NEON_NEW RG::RenderGraph);
        *this = Other;
    }

    Camera& Camera::operator=(
        const Camera& Other)
    {
        if (this != &Other)
        {
            RenderGraph->Reset(Other.RenderGraph.get());
            Viewport       = Other.Viewport;
            CullMask       = Other.CullMask;
            RenderPriority = Other.RenderPriority;
            Type           = Other.Type;
        }
        return *this;
    }

    Camera::Camera(
        Camera&& Other) :
        RenderGraph(std::move(Other.RenderGraph)),
        Viewport(std::move(Other.Viewport)),
        CullMask(Other.CullMask),
        RenderPriority(Other.RenderPriority),
        Type(Other.Type)
    {
    }

    Camera& Camera::operator=(
        Camera&& Other)
    {
        if (this != &Other)
        {
            RenderGraph    = std::move(Other.RenderGraph);
            Viewport       = std::move(Other.Viewport);
            CullMask       = Other.CullMask;
            RenderPriority = Other.RenderPriority;
            Type           = Other.Type;
        }
        return *this;
    }

    //

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
        {
            return glm::perspectiveFov(
                FieldOfView,
                Width,
                Height,
                NearPlane,
                FarPlane);
        }
        case CameraType::Orthographic:
        {
            float XAxisMultiplier = 1.f;
            float YAxisMultiplier = 1.f / AspectRatio();

            if (!MaintainXFov)
            {
                std::swap(XAxisMultiplier, YAxisMultiplier);
            }

            float HalfSize = OrthographicSize / 2.f;
            return glm::ortho(
                -HalfSize * XAxisMultiplier,
                HalfSize * XAxisMultiplier,
                -HalfSize * YAxisMultiplier,
                HalfSize * YAxisMultiplier,
                NearPlane,
                FarPlane);
        }
        default:
            return Mat::Identity<Matrix4x4>;
        }
    }

    Matrix4x4 Camera::ProjectionMatrix() const
    {
        return Viewport.ProjectionMatrix(Type);
    }

    RG::RenderGraph* Camera::GetRenderGraph() const
    {
        return RenderGraph.get();
    }
} // namespace Neon::Scene::Component