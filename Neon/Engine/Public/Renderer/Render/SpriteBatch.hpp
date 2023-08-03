#pragma once

#include <Renderer/Material/Material.hpp>
#include <Renderer/Render/PrimitiveBatch.hpp>

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
#if 0
    class SpriteBatcher : public Impl::PrimitiveBatch
    {
        struct SpriteVertex
        {
            Vector2 Position;
            Vector2 TexCoord;
            int     SpriteIndex;
        };

    public:
        /// <summary>
        /// Called before final drawing.
        /// </summary>
        static constexpr size_t VertexStride = sizeof(SpriteVertex);
        static constexpr size_t IndexStride  = sizeof(uint16_t);

        static constexpr size_t MaxVerticesCount = 4096;

        SpriteBatcher(
            uint32_t VerticesCount = MaxVerticesCount,
            uint32_t IndexCount    = MaxVerticesCount * 3) :
            Impl::PrimitiveBatch(
                VertexStride,
                VerticesCount,
                IndexStride,
                IndexCount)
        {
        }

        /// <summary>
        /// Begin drawing.
        /// </summary>
        void Begin(
            RHI::IGraphicsCommandList* CommandList,
            RHI::PrimitiveTopology     Topology)
        {
            return Impl::PrimitiveBatch::Begin(
                CommandList,
                Topology,
                true);
        }

        /// <summary>
        /// Called before final drawing.
        /// </summary>
        void OnDraw() override;

        /// <summary>
        /// End drawing.
        /// </summary>
        void End()
        {
            Impl::PrimitiveBatch::End();
        }

    public:
        /// <summary>
        /// Draw a rectangle.
        /// </summary>
        void DrawSprite(
            const Scene::Component::Transform& Transform,
            const Scene::Component::Sprite&    Sprite);

    private:
        Ptr<RHI::IUploadBuffer> m_PerDataBuffer;
        MaterialTable           m_MaterialInstances;
    };
#endif

    class SpriteBatch
    {
    public:
        SpriteBatch();

        /// <summary>
        /// Update camera buffer.
        /// </summary>
        void SetCameraBuffer(
            const Ptr<RHI::IUploadBuffer>& Buffer);

        /// <summary>
        /// Begins drawing.
        /// </summary>
        void Begin(
            RHI::IGraphicsCommandList* CommandList);

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
        /// Creates the vertex, per object and index buffers.
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

        RHI::IPipelineState* m_PipelineState = nullptr;
        MaterialTable        m_MaterialInstances;
    };
} // namespace Neon::Renderer