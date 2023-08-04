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
    struct BatchSpriteVertex
    {
        Vector2  Position;
        Vector2  TexCoord;
        uint32_t SpriteIndex;
    };

    class SpriteBatcher : protected PrimitiveBatch16<BatchSpriteVertex>
    {
        struct PerObjectData
        {
            Matrix4x4 World;
            Vector4   Color;
        };

        using BatchBaseClass = PrimitiveBatch16<BatchSpriteVertex>;

    public:
        static constexpr size_t MaxSpritesCount = 2048;

        SpriteBatcher(
            uint32_t SpritesCount = MaxSpritesCount) :
            BatchBaseClass(
                SpritesCount * 4,
                SpritesCount * 6),
            m_PerObjectBuffer(SpritesCount * sizeof(PerObjectData))
        {
        }

        /// <summary>
        /// Update camera buffer.
        /// </summary>
        void SetCameraBuffer(
            const Ptr<RHI::IUploadBuffer>& Buffer);

    protected:
        /// <summary>
        /// Called before final drawing.
        /// </summary>
        void OnDraw() override;

        /// <summary>
        /// Called after drawing.
        /// </summary>
        void OnReset() override;

    public:
        /// <summary>
        /// Begin drawing.
        /// </summary>
        void Begin(
            RHI::IGraphicsCommandList* CommandList)
        {
            Impl::PrimitiveBatch::Begin(CommandList, RHI::PrimitiveTopology::TriangleList, true);
        }

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
        void Draw(
            const Scene::Component::Transform& Transform,
            const Scene::Component::Sprite&    Sprite);

    private:
        FrameBuffer   m_PerObjectBuffer;
        MaterialTable m_MaterialInstances;

        Ptr<RHI::IUploadBuffer> m_CameraBuffer;
    };

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