#include <EnginePCH.hpp>
#include <Renderer/Render/PrimitiveBatch.hpp>
#include <RHI/Resource/Views/Shader.hpp>

#include <Log/Logger.hpp>

namespace Neon::Renderer::Impl
{
    /// <summary>
    /// Get the size of an index buffer.
    /// </summary>
    [[nodiscard]] size_t GetIndexBufferSize(
        size_t Count,
        bool   Is32Bit)
    {
        return Count * (Is32Bit ? sizeof(uint32_t) : sizeof(uint16_t));
    }

    /// <summary>
    /// Get the size of an vertex buffer.
    /// </summary>
    [[nodiscard]] size_t GetVertexBufferSize(
        size_t Count,
        size_t Stride)
    {
        return Count * Stride;
    }

    //

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

    void PrimitiveBatch::Begin(
        RHI::IGraphicsCommandList* CommandList,
        RHI::PrimitiveTopology     Topology,
        bool                       Indexed)
    {
        NEON_ASSERT(CommandList != nullptr, "CommandList is null");

        m_DrawingIndexed = Indexed;
        m_CommandList    = CommandList;
        m_Topology       = Topology;
    }

    void PrimitiveBatch::Draw(
        uint32_t VertexSize,
        void**   OutVertices,
        uint32_t IndexSize,
        void**   OutIndices)
    {
        NEON_ASSERT(!m_DrawingIndexed || (m_DrawingIndexed && IndexSize && OutIndices && m_IndexBuffer), "Drawing indexed without index buffer");

        if (!m_VertexBuffer.Reserve(VertexSize) || (m_IndexBuffer && !m_IndexBuffer.Reserve(IndexSize)))
        {
            End();

            m_VertexBuffer.Reset();
            m_IndexBuffer.Reset();

            Begin(m_CommandList, m_Topology, m_DrawingIndexed);
        }

        *OutVertices = m_VertexBuffer.AllocateData(VertexSize);
        m_VerticesSize += VertexSize;
        if (m_DrawingIndexed)
        {

            *OutIndices = m_IndexBuffer.AllocateData(IndexSize);
            m_IndicesSize += IndexSize;
        }
    }

    void PrimitiveBatch::End()
    {
        OnDraw();

        m_CommandList->SetPrimitiveTopology(m_Topology);

        RHI::Views::Vertex VtxView;
        VtxView.Append(m_VertexBuffer.GetHandleFor(m_VerticesSize), m_VertexStride, m_VerticesSize);

        m_CommandList->SetVertexBuffer(0, VtxView);

        if (m_DrawingIndexed)
        {
            RHI::Views::Index IdxView(m_IndexBuffer.GetHandleFor(m_IndicesSize), m_IndicesSize, m_Is32BitIndex);

            m_CommandList->SetIndexBuffer(IdxView);
            m_CommandList->Draw(RHI::DrawIndexArgs{ .IndexCountPerInstance = m_IndicesSize / m_VertexStride });
        }
        else
        {
            m_CommandList->Draw(RHI::DrawArgs{ .VertexCountPerInstance = m_VerticesSize / m_VertexStride });
        }

        m_VerticesSize = 0;
        m_IndicesSize  = 0;
    }
} // namespace Neon::Renderer::Impl
