#pragma once

#include <RHI/Resource/Resource.hpp>

#include <Math/Vector.hpp>
#include <Math/Rect.hpp>
#include <Math/Colors.hpp>

#include <vector>

namespace Neon::Renderer
{
    class SpriteBatch
    {
    public:
        struct QuadCommand
        {
            Vector2 Position;
            Vector2 Size;
            float   Rotation = 0.f;
            float   Depth    = 0.f;
            Color4  Color    = Colors::White;

            Ptr<RHI::ITexture> Texture;
            RectF              TexCoord = RectF(Vec::Zero<Vector2>, Vec::One<Vector2>);
        };

        SpriteBatch();

        /// <summary>
        /// Enqueues a quad to be drawn.
        /// </summary>
        void Draw(
            const QuadCommand& Quad);

    private:
        UPtr<RHI::IUploadBuffer> m_VertexBuffers;
        UPtr<RHI::IUploadBuffer> m_IndexBuffer;
        size_t                   m_DrawCount = 0;
    };
} // namespace Neon::Renderer