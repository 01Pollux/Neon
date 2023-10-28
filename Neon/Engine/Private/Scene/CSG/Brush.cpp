#include <EnginePCH.hpp>
#include <Scene/CSG/Brush.hpp>
#include <Math/Matrix.hpp>
#include <Mdl/Submesh.hpp>

namespace Neon::Scene::CSG
{
    Brush::Brush(
        const FaceList& Faces,
        MaterialList    Materials,
        const void*     Indices,
        size_t          IndicesCount,
        bool            Is16BitsIndex) :
        m_Materials(std::move(Materials)),
        m_VerticesCount(uint32_t(Faces.size() * std::size(Faces[0].Vertices))),
        m_IndicesCount(uint32_t(IndicesCount)),
        m_Is16BitsIndex(Is16BitsIndex)
    {
        BuildAABB(Faces);
        BuildGpuBuffer(Faces, Indices, IndicesCount);
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
        const FaceList& Faces)
    {
        Vector3 Min(std::numeric_limits<float>::max()),
            Max(std::numeric_limits<float>::lowest());

        for (auto& Face : Faces)
        {
            for (auto& Vertex : Face.Vertices)
            {
                Min = glm::min(Min, Vertex.Position);
                Max = glm::max(Max, Vertex.Position);
            }
        }

        m_AABB = {
            .Center  = (Max + Min) * 0.5f,
            .Extents = (Max - Min) * 0.5f
        };
    }

    void Brush::BuildGpuBuffer(
        const FaceList& Faces,
        const void*     Indices,
        size_t          IndicesCount)
    {
        uint32_t VerticesCount = GetVerticesCount();
        uint32_t IndexSize     = Is16BitsIndex() ? sizeof(uint16_t) : sizeof(uint32_t);

        m_Buffer = RHI::UBufferPoolHandle(
            VerticesCount * sizeof(Mdl::MeshVertex) + IndicesCount * IndexSize,
            alignof(Mdl::MeshVertex),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        auto BufferPtr = m_Buffer.AsUpload().Map() + m_Buffer.Offset;

        for (auto& Face : Faces)
        {
            for (size_t i = 0; i < std::size(Face.Vertices); i++)
            {
                *std::bit_cast<Mdl::MeshVertex*>(BufferPtr) = Face.Vertices[i];
                BufferPtr += sizeof(Mdl::MeshVertex);
            }
        }

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