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
        m_VertexMaxCount(VetexCount),
        m_IndexMaxCount(IndexCount),
        m_Is32BitIndex(IndexStride == sizeof(uint32_t))
    {
        NEON_ASSERT(m_VertexStride > 0, "Vertex stride is 0");
        NEON_ASSERT(m_VertexCount > 0, "Vertex count is 0");
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
        uint32_t VertexCount,
        void**   OutVertices,
        uint32_t IndexCount,
        void**   OutIndices)
    {
        NEON_ASSERT(!m_DrawingIndexed || (m_DrawingIndexed && OutIndices && OutIndices), "Drawing indexed without index buffer");

        bool Overflow = false;
        // Check if we already have a vertex buffer
        if (m_VertexBuffer)
        {
            // Check if we have enough space in the vertex buffer
            const size_t MaxPossibleVertices = m_VertexMaxCount - m_VertexOffset;

            const size_t MaxVerticesSize = GetVertexBufferSize(MaxPossibleVertices, m_VertexStride);
            const size_t MaxRequiredSize = GetVertexBufferSize(m_VertexCount, m_VertexStride);

            Overflow = (MaxRequiredSize > MaxVerticesSize);
            // If we do have enough space, check if we have enough space in the index buffer if possible
            if (!Overflow && m_DrawingIndexed)
            {
                const size_t MaxPossibleIndices = m_IndexMaxCount - m_IndexOffset;

                const size_t MaxIndicesSize     = GetIndexBufferSize(MaxPossibleIndices, m_Is32BitIndex);
                const size_t MaxRequiredIdxSize = GetIndexBufferSize(m_IndexCount, m_Is32BitIndex);

                Overflow = MaxRequiredIdxSize > MaxIndicesSize;
            }
        }

        // Reset on overflow
        if (Overflow)
        {
            End();

            // Unmap buffers
            m_VertexBuffer->Unmap();
            if (m_IndexBuffer)
            {
                m_IndexBuffer->Unmap();
            }

            m_VertexBuffer = nullptr;
            m_IndexBuffer  = nullptr;

            Begin(m_CommandList, m_Topology, m_DrawingIndexed);
        }

        // Reallocate vertex buffer if needed.
        if (!m_VertexBuffer)
        {
            m_VertexBuffer.reset(RHI::IUploadBuffer::Create({ .Size = GetVertexBufferSize(m_VertexMaxCount, m_VertexStride) }));
            m_VertexOffset   = 0;
            m_MappedVertices = m_VertexBuffer->Map();
        }

        // Reallocate index buffer if needed.
        if (m_DrawingIndexed && !m_IndexBuffer)
        {
            m_IndexBuffer.reset(RHI::IUploadBuffer::Create({ .Size = GetIndexBufferSize(m_IndexMaxCount, m_Is32BitIndex) }));
            m_IndexOffset   = 0;
            m_MappedIndices = m_IndexBuffer->Map();
        }

        // Load address of the vertex buffer.
        {
            *OutVertices = static_cast<uint8_t*>(m_MappedVertices) + GetVertexBufferSize(m_VertexOffset, m_VertexStride);
            m_VertexOffset += VertexCount;
        }

        // Load address of the index buffer.
        if (m_IndexBuffer && OutIndices)
        {
            NEON_ASSERT(IndexCount, "Index count is 0");
            *OutIndices = static_cast<uint8_t*>(m_MappedIndices) + GetIndexBufferSize(m_IndexOffset, m_Is32BitIndex);
            m_IndexOffset += IndexCount;
        }
    }

    void PrimitiveBatch::End()
    {
        m_CommandList->SetPrimitiveTopology(m_Topology);

        RHI::Views::Vertex VtxView;
        VtxView.Append(m_VertexBuffer->GetHandle(), m_VertexStride, m_VertexCount);

        m_CommandList->SetVertexBuffer(0, VtxView);

        if (m_DrawingIndexed)
        {
            RHI::Views::Index IdxView(m_IndexBuffer->GetHandle(), m_IndexCount, m_Is32BitIndex);

            m_CommandList->SetIndexBuffer(IdxView);
            m_CommandList->Draw(RHI::DrawIndexArgs{ .IndexCountPerInstance = m_IndexCount });
        }
        else
        {
            m_CommandList->Draw(RHI::DrawArgs{ .VertexCountPerInstance = m_VertexCount });
        }

        // Reusing the same buffers, so we need to offset the next draw call.
        m_VertexOffset += m_VertexCount;
        m_IndexOffset += m_IndexCount;

        // Reset the vertex and index count.
        m_VertexCount = 0;
        m_IndexCount  = 0;
    }
} // namespace Neon::Renderer::Impl
