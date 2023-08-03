#pragma once

#include <Renderer/Render/FrameBuffer.hpp>
#include <RHI/Commands/List.hpp>

#include <Math/Vector.hpp>
#include <Math/Rect.hpp>
#include <Math/Colors.hpp>

namespace Neon::Renderer
{
    namespace Impl
    {
        class PrimitiveBatch
        {
        public:
            NEON_CLASS_NO_COPYMOVE(PrimitiveBatch);
            virtual ~PrimitiveBatch() = default;

            /// <summary>
            /// Called before final drawing.
            /// </summary>
            virtual void OnDraw()
            {
            }

        protected:
            PrimitiveBatch(
                uint32_t VertexStride,
                uint32_t VetexCount,
                uint32_t IndexStride,
                uint32_t IndexCount);

            /// <summary>
            /// Begin drawing.
            /// </summary>
            void Begin(
                RHI::IGraphicsCommandList* CommandList,
                RHI::PrimitiveTopology     Topology,
                bool                       Indexed = false);

            /// <summary>
            /// Draw a primitive.
            /// </summary>
            void Draw(
                uint32_t VertexSize,
                void**   OutVertices,
                uint32_t IndexSize,
                void**   OutIndices);

            /// <summary>
            /// End drawing.
            /// </summary>
            void End();

        private:
            RHI::IGraphicsCommandList* m_CommandList = nullptr;

            /// <summary>
            /// Stride of the vertex buffer.
            /// </summary>
            uint32_t m_VertexStride;

            FrameBuffer m_VertexBuffer;
            FrameBuffer m_IndexBuffer;

            uint32_t m_VerticesSize = 0;
            uint32_t m_IndicesSize  = 0;

            RHI::PrimitiveTopology m_Topology;

            bool m_DrawingIndexed : 1 = false;
            bool m_Is32BitIndex   : 1 = false;
        };
    } // namespace Impl
} // namespace Neon::Renderer