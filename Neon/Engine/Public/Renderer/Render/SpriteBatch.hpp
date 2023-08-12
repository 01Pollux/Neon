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
            Matrix4x4 TextureTransform;
            Vector4   Color;
            int       TextureIndex;
            int       _Padding[3];
        };

        static_assert(!(sizeof(PerObjectData) % 16), "PerObjectData size must be a multiple of 16");
        using BatchBaseClass = PrimitiveBatch16<BatchSpriteVertex>;

    public:
        static constexpr size_t MaxSpritesCount = 50'000;

        explicit SpriteBatcher(
            uint32_t SpritesCount = MaxSpritesCount) :
            BatchBaseClass(
                SpritesCount * 4,
                SpritesCount * 6),
            m_PerObjectBuffer(SpritesCount * sizeof(PerObjectData))
        {
        }

        /// <summary>
        /// Set camera's buffer.
        /// </summary>
        void SetCameraBuffer(
            RHI::GpuResourceHandle Buffer);

    protected:
        /// <summary>
        /// Called before drawing.
        /// </summary>
        void OnBegin() override;

        /// <summary>
        /// Called before final drawing.
        /// </summary>
        void OnDraw() override;

        /// <summary>
        /// Called after drawing.
        /// </summary>
        void OnEnd() override;

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

        RHI::GpuResourceHandle m_CameraBuffer;
    };
} // namespace Neon::Renderer