#include <EnginePCH.hpp>
#include <Scene/CSG/Brush.hpp>
#include <Math/Matrix.hpp>
#include <Mdl/Submesh.hpp>

namespace Neon::Scene::CSG
{
    Brush::Brush(
        const VertexList& Vertices,
        MaterialList      Materials,
        const void*       Indices,
        size_t            IndicesCount,
        bool              Is16BitsIndex) :
        m_Materials(std::move(Materials)),
        m_VerticesCount(uint32_t(Vertices.size())),
        m_IndicesCount(uint32_t(IndicesCount)),
        m_Is16BitsIndex(Is16BitsIndex)
    {
        BuildAABB(Vertices);
        BuildGpuBuffer(Vertices, Indices, IndicesCount);
    }

    //

    RHI::GpuResourceHandle Brush::GetVertexBuffer() const
    {
        return m_Buffer.GetGpuHandle();
    }

    RHI::GpuResourceHandle Brush::GetIndexBuffer() const
    {
        return m_Buffer.GetGpuHandle(GetVerticesCount() * sizeof(Mdl::MeshVertex));
    }

    uint32_t Brush::GetVerticesCount() const
    {
        return m_VerticesCount;
    }

    uint32_t Brush::GetIndicesCount() const
    {
        return m_IndicesCount;
    }

    bool Brush::Is16BitsIndex() const
    {
        return m_Is16BitsIndex;
    }

    //

    void Brush::BuildAABB(
        const VertexList& Vertices)
    {
        Vector3 Min(std::numeric_limits<float>::max()),
            Max(std::numeric_limits<float>::lowest());

        for (auto& Vertex : Vertices)
        {
            Min = glm::min(Min, Vertex.Position);
            Max = glm::max(Max, Vertex.Position);
        }

        m_AABB = {
            .Center  = (Max + Min) * 0.5f,
            .Extents = (Max - Min) * 0.5f
        };
    }

    void Brush::BuildGpuBuffer(
        const VertexList& Vertices,
        const void*       Indices,
        size_t            IndicesCount)
    {
        uint32_t VerticesCount = GetVerticesCount();
        uint32_t IndexSize     = Is16BitsIndex() ? sizeof(uint16_t) : sizeof(uint32_t);

        m_Buffer = RHI::UBufferPoolHandle(
            VerticesCount * sizeof(Mdl::MeshVertex) + IndicesCount * IndexSize,
            alignof(Mdl::MeshVertex),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        auto BufferPtr = m_Buffer.AsUpload().Map() + m_Buffer.Offset;

        std::copy_n(std::bit_cast<Mdl::MeshVertex*>(Vertices.data()), VerticesCount, std::bit_cast<Mdl::MeshVertex*>(BufferPtr));
        BufferPtr += VerticesCount * sizeof(Mdl::MeshVertex);

        if (IndexSize == sizeof(uint16_t))
        {
            std::copy_n(std::bit_cast<uint16_t*>(Indices), IndicesCount, std::bit_cast<uint16_t*>(BufferPtr));
        }
        else
        {
            std::copy_n(std::bit_cast<uint32_t*>(Indices), IndicesCount, std::bit_cast<uint32_t*>(BufferPtr));
        }

        m_Buffer.AsUpload().Unmap();
    }
} // namespace Neon::Scene::CSG