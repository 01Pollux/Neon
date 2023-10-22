#pragma once

#include <Core/Neon.hpp>
#include <Math/Matrix.hpp>

#include <Scene/Component/Component.hpp>
#include <RenderGraph/Graph.hpp>

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
        NEON_EXPORT_FLECS_COMPONENT(Camera, "Camera")
        {
        }

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

        Camera() = default;
        Camera(
            CameraType Type);

        /// <summary>
        /// Get the projection matrix of the viewport.
        /// </summary>
        [[nodiscard]] Matrix4x4 ProjectionMatrix() const;

    private:
        /// <summary>
        /// The render graph of the camera.
        /// </summary>
        Ptr<RG::RenderGraph> RenderGraph;

    public:
        /// <summary>
        /// Create a new render graph for the camera.
        /// </summary>
        RG::RenderGraph* NewRenderGraph(
            const flecs::entity& OwningEntity);

        /// <summary>
        /// Get the render graph of the camera.
        /// </summary>
        RG::RenderGraph* GetRenderGraph() const;

        /// <summary>
        /// The viewport of the camera.
        /// </summary>
        Viewport Viewport;

        /// <summary>
        /// The culling mask of the camera.
        /// </summary>
        uint32_t CullMask = 0xFFFFFFFF;

        /// <summary>
        /// The render order of the camera.
        /// </summary>
        int RenderPriority = 0;

        /// <summary>
        /// The type of the camera.
        /// </summary>
        CameraType Type = CameraType::Perspective;
    };
} // namespace Neon::Scene::Component
