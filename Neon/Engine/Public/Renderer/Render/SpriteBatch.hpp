#pragma once

#include <RHI/Resource/Resource.hpp>
#include <RHI/Resource/Views/ShaderResource.hpp>
#include <Renderer/Material/Material.hpp>

#include <Math/Vector.hpp>
#include <Math/Rect.hpp>
#include <Math/Colors.hpp>

namespace Neon
{
    namespace RHI
    {
        class ISwapchain;
        class IPipelineState;
        class IGraphicsCommandList;
    } // namespace RHI
    namespace Renderer
    {
        class IMaterial;
    }
    namespace Scene::Component
    {
        struct Sprite;
        struct Camera;
        struct Transform;
    } // namespace Scene::Component
} // namespace Neon

namespace Neon::Renderer
{
    class SpriteBatch
    {
    public:
        SpriteBatch(
            size_t MaxSpritePageCount);

        /// <summary>
        /// Update camera buffer.
        /// </summary>
        void SetCameraBuffer(
            const Ptr<RHI::IUploadBuffer>& Buffer);

        /// <summary>
        /// Begins drawing.
        /// </summary>
        void Begin(
            RHI::IGraphicsCommandList* CommandList,
            size_t                     SpritePageCount);

        /// <summary>
        /// Enqueues a quad to be drawn.
        /// </summary>
        void Draw(
            const Scene::Component::Transform& Transform,
            const Scene::Component::Sprite&    Sprite);

        /// <summary>
        /// Ends drawing.
        /// </summary>
        void End();

    private:
        /// <summary>
        /// Creates the vertex and index buffers.
        /// </summary>
        void CreateBuffers();

    private:
        RHI::IGraphicsCommandList* m_CommandList = nullptr;

        Ptr<RHI::IUploadBuffer> m_CameraBuffer;

        Ptr<RHI::IUploadBuffer> m_PerDataBuffer;
        Ptr<RHI::IUploadBuffer> m_VertexBuffer;
        Ptr<RHI::IUploadBuffer> m_IndexBuffer;

        uint8_t* m_VertexBufferPtr  = nullptr;
        uint8_t* m_PerDataBufferPtr = nullptr;

        uint32_t m_VerticesCount = 0;
        uint32_t m_DrawCount     = 0;

        MaterialTable        m_MaterialTable;
        RHI::IPipelineState* m_PipelineState = nullptr;
    };
} // namespace Neon::Renderer