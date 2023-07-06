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
} // namespace Neon

namespace Neon::Renderer
{
    class SpriteBatch
    {
    public:
        struct QuadCommand
        {
            Vector3 Position;
            Vector2 Size;
            float   Rotation = 0.f;
            float   Depth    = 0.f;
            Color4  Color    = Colors::White;

            Ptr<RHI::ITexture> Texture;
            RectF              TexCoord = RectF(Vec::Zero<Vector2>, Vec::One<Vector2>);
        };

    public:
        SpriteBatch(
            Ptr<IMaterial>   SpriteMaterial,
            RHI::ISwapchain* Swapchain);

        /// <summary>
        /// Begins drawing.
        /// </summary>
        void Begin(
            RHI::IGraphicsCommandList* CommandList);

        /// <summary>
        /// Enqueues a quad to be drawn.
        /// </summary>
        void Draw(
            const QuadCommand& Quad);

        /// <summary>
        /// Ends drawing.
        /// </summary>
        void End();

    private:
        /// <summary>
        /// Creates the vertex and index buffers.
        /// </summary>
        void CreateBuffers(
            RHI::ISwapchain* Swapchain);

    private:
        Ptr<IMaterial> m_SpriteMaterial;

        RHI::IGraphicsCommandList* m_CommandList = nullptr;

        RHI::Views::ShaderResource m_ResourceView;
        RHI::Views::Generic        m_SamplerView;

        UPtr<RHI::IUploadBuffer> m_VertexBuffer;
        UPtr<RHI::IUploadBuffer> m_IndexBuffer;

        uint8_t* m_VertexBufferPtr = nullptr;

        uint32_t m_VerticesCount = 0;
        uint32_t m_DrawCount     = 0;
        uint32_t m_TextureCount  = 0;
    };
} // namespace Neon::Renderer