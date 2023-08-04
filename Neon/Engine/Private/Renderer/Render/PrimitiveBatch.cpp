#include <EnginePCH.hpp>
#include <Renderer/Render/PrimitiveBatch.hpp>
#include <RHI/Resource/Views/Shader.hpp>

#include <Log/Logger.hpp>

namespace Neon::Renderer::Impl
{
    PrimitiveBatch::PrimitiveBatch(
        uint32_t VertexStride,
        uint32_t VetexCount,
        uint32_t IndexStride,
        uint32_t IndexCount) :
        m_VertexStride(VertexStride),
        m_VertexBuffer(VertexStride * VetexCount),
        m_IndexBuffer(IndexStride * IndexCount),
        m_Is32BitIndex(IndexStride == sizeof(uint32_t))
    {
        NEON_ASSERT(VertexStride > 0, "Vertex stride is 0");
        NEON_ASSERT(VetexCount > 0, "Vertex count is 0");
    }

    void PrimitiveBatch::Reset()
    {
        End();

        m_VertexBuffer.Reset();
        m_IndexBuffer.Reset();

        Begin(m_CommandList, m_Topology, m_DrawingIndexed);
    }

    void PrimitiveBatch::Begin(
        RHI::IGraphicsCommandList* CommandList,
        RHI::PrimitiveTopology     Topology,
        bool                       Indexed)
    {
        NEON_ASSERT(CommandList != nullptr, "CommandList is null");
        NEON_ASSERT(Topology != RHI::PrimitiveTopology::Undefined, "Topology is undefined");

        m_DrawingIndexed = Indexed;
        m_CommandList    = CommandList;
        m_Topology       = Topology;
    }

    uint32_t PrimitiveBatch::Draw(
        uint32_t VertexSize,
        void**   OutVertices,
        uint32_t IndexSize,
        void**   OutIndices)
    {
        NEON_ASSERT(!m_DrawingIndexed || (m_DrawingIndexed && IndexSize && OutIndices && m_IndexBuffer), "Drawing indexed without index buffer");
        NEON_ASSERT(VertexSize && OutVertices && m_VertexBuffer, "Drawing without vertex buffer");
        NEON_ASSERT((VertexSize % m_VertexStride) == 0, "VertexSize is not aligned to m_VertexStride");

        if (!m_VertexBuffer.Reserve(VertexSize) || (m_IndexBuffer && !m_IndexBuffer.Reserve(IndexSize)))
        {
            Reset();
        }

        uint32_t VertexOffset = m_VerticesSize;

        *OutVertices = m_VertexBuffer.AllocateData(VertexSize);
        m_VerticesSize += VertexSize;
        if (m_DrawingIndexed)
        {
            *OutIndices = m_IndexBuffer.AllocateData(IndexSize);
            m_IndicesSize += IndexSize;
        }

        return VertexOffset / m_VertexStride;
    }

    void PrimitiveBatch::End()
    {
        if (m_VerticesSize)
        {
            m_CommandList->SetPrimitiveTopology(m_Topology);

            RHI::Views::Vertex VtxView;
            VtxView.Append(m_VertexBuffer.GetHandleForSize(m_VerticesSize), m_VertexStride, m_VerticesSize);
            m_CommandList->SetVertexBuffer(0, VtxView);

            if (m_DrawingIndexed)
            {
                RHI::Views::Index IdxView(m_IndexBuffer.GetHandleForSize(m_IndicesSize), m_IndicesSize, m_Is32BitIndex);
                m_CommandList->SetIndexBuffer(IdxView);

                OnDraw();

                uint32_t IndexCount = m_IndicesSize / (m_Is32BitIndex ? sizeof(uint32_t) : sizeof(uint16_t));
                m_CommandList->Draw(RHI::DrawIndexArgs{ .IndexCountPerInstance = IndexCount });
            }
            else
            {
                OnDraw();

                uint32_t VertexCount = m_VerticesSize / m_VertexStride;
                m_CommandList->Draw(RHI::DrawArgs{ .VertexCountPerInstance = VertexCount });
            }

            OnReset();
            m_VerticesSize = 0;
            m_IndicesSize  = 0;
        }
    }
} // namespace Neon::Renderer::Impl
