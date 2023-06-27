#include <EnginePCH.hpp>
#include <Renderer/Render/SpriteBatch.hpp>
#include <RHI/Resource/MappedBuffer.hpp>

namespace Neon::Renderer
{
    namespace SpriteBatchConstants
    {
        constexpr size_t MaxQuads    = 10'000;
        constexpr size_t MaxVertices = MaxQuads * 4;
        constexpr size_t MaxIndices  = MaxQuads * 6;

        struct Vertex
        {
            Vector3 Position;
            Color4  Color;
            Vector2 TexCoord;
        };

        struct Quad
        {
            Vertex TopLeft;
            Vertex TopRight;
            Vertex BottomLeft;
            Vertex BottomRight;
        };

    } // namespace SpriteBatchConstants

    SpriteBatch::SpriteBatch()
    {
    }

    void SpriteBatch::Draw(
        const QuadCommand& Quad)
    {
    }
} // namespace Neon::Renderer