#pragma once

#include <Core/Neon.hpp>
#include <flecs/flecs.h>
#include <Math/Matrix.hpp>

#include <Scene/Component/Component.hpp>

namespace Neon::RG
{
    class RenderGraph;
} // namespace Neon::RG

namespace Neon::RHI
{
    class IUploadBuffer;
} // namespace Neon::RHI

namespace Neon::Scene::Component
{
    struct CameraFrameData
    {
        alignas(16) Matrix4x4 World;

        alignas(16) Matrix4x4 View;
        alignas(16) Matrix4x4 Projection;
        alignas(16) Matrix4x4 ViewProjection;

        alignas(16) Matrix4x4 ViewInverse;
        alignas(16) Matrix4x4 ProjectionInverse;
        alignas(16) Matrix4x4 ViewProjectionInverse;

        float EngineTime;
        float GameTime;
        float DeltaTime;
    };

    enum class CameraType : uint8_t
    {
        Perspective,
        Orthographic
    };

    struct Camera
    {
        struct Viewport
        {
            /// <summary>
            /// The near and far plane of the viewport.
            /// </summary>
            float NearPlane = 0.1f;

            /// <summary>
            /// The far plane of the viewport.
            /// </summary>
            float FarPlane = 1000.0f;

            /// <summary>
            /// The field of view of the viewport.
            /// </summary>
            float FieldOfView = 45.0f;

            /// <summary>
            /// The width and height of the viewport.
            /// </summary>
            float Width = 800.f;

            /// <summary>
            /// The width and height of the viewport.
            /// </summary>
            float Height = 600.f;

            /// <summary>
            /// The orthographic size of the viewport.
            /// </summary>
            float OrthographicSize = 1.0f;

            /// <summary>
            /// Whether the viewport is using client width and height.
            /// </summary>
            bool ClientWidth : 1 = true;

            /// <summary>
            /// Whether the viewport is using client width and height.
            /// </summary>
            bool ClientHeight : 1 = true;

            /// <summary>
            /// Maintain the x field of view when resizing.
            /// </summary>
            bool MaintainXFov : 1 = true;

            /// <summary>
            /// Get the aspect ratio of the viewport.
            /// </summary>
            [[nodiscard]] float AspectRatio() const;

            /// <summary>
            /// Get the projection matrix of the viewport.
            /// </summary>
            [[nodiscard]] Matrix4x4 ProjectionMatrix(
                CameraType Type) const;
        };

        Camera();
        Camera(
            UPtr<RG::RenderGraph> RenderGraph,
            CameraType            Type);

        NEON_CLASS_NO_COPY(Camera);
        NEON_CLASS_MOVE_DECL(Camera);

        ~Camera();

        /// <summary>
        /// Get the projection matrix of the viewport.
        /// </summary>
        [[nodiscard]] Matrix4x4 ProjectionMatrix() const;

        /// <summary>
        /// Get the view matrix of the viewport.
        /// </summary>
        [[nodiscard]] Matrix4x4 ViewMatrix(
            const flecs::entity& OwningEntity) const;

        /// <summary>
        /// Get the view matrix of the viewport.
        /// </summary>
        [[nodiscard]] Matrix4x4 ViewProjectionMatrix(
            const flecs::entity& OwningEntity) const;

        NEON_EXPORT_COMPONENT();

        /// <summary>
        /// The render graph of the camera.
        /// </summary>
        UPtr<RG::RenderGraph> RenderGraph;

        /// <summary>
        /// The viewport of the camera.
        /// </summary>
        Viewport Viewport;

        /// <summary>
        /// The position for the camera to look at.
        /// </summary>
        Vector3 LookAt = Vec::Zero<Vector3>;

        /// <summary>
        /// The culling mask of the camera.
        /// </summary>
        uint32_t CullMask = 0xFFFFFFFF;

        /// <summary>
        /// The render order of the camera.
        /// </summary>
        uint32_t RenderPriority = 0;

        /// <summary>
        /// The type of the camera.
        /// </summary>
        CameraType Type = CameraType::Perspective;

        //

        /// <summary>
        /// The graphics buffer of the camera.
        /// Structured as CameraFrameData.
        /// </summary>
        Ptr<RHI::IUploadBuffer> GraphicsBuffer;
    };

    struct MainCamera
    {
        Actor Target;
    };
} // namespace Neon::Scene::Component
