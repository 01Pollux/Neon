#pragma once

#include <Scene/Component/Component.hpp>
#include <flecs/flecs.h>
#include <Math/Matrix.hpp>

namespace Neon::RG
{
    class RenderGraph;
} // namespace Neon::RG

namespace Neon::Scene::Component
{
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

        /// <summary>
        /// Get the projection matrix of the viewport.
        /// </summary>
        [[nodiscard]] Matrix4x4 ProjectionMatrix() const;

        /// <summary>
        /// Get the view matrix of the viewport.
        /// </summary>
        [[nodiscard]] static Matrix4x4 ViewMatrix(
            const flecs::entity& OwningEntity,
            const Vector3&       Position);

        /// <summary>
        /// Get the view matrix of the viewport.
        /// </summary>
        [[nodiscard]] static Matrix4x4 ViewProjectionMatrix(
            const flecs::entity& OwningEntity,
            const Vector3&       Position);

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
        /// The culling mask of the camera.
        /// </summary>
        uint32_t CullMask = 0xFFFFFFFF;

        /// <summary>
        /// The type of the camera.
        /// </summary>
        CameraType Type = CameraType::Perspective;
    };
} // namespace Neon::Scene::Component