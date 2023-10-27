#include <EnginePCH.hpp>
#include <Scene/CSG/Brush.hpp>
#include <Math/Matrix.hpp>
#include <Mdl/Submesh.hpp>

namespace Neon::Scene::CSG
{
    Brush::Brush(
        FaceList     Faces,
        MaterialList Materials) :
        m_Faces(std::move(Faces)),
        m_Materials(std::move(Materials))
    {
        BuildAABB();
    }

    Brush::Brush(
        const Brush&           Brush,
        const TransformMatrix& Transform) :
        m_Faces(Brush.m_Faces),
        m_Materials(Brush.m_Materials)
    {
        auto Matrix = Transform.ToMat4x4();
        for (auto& Face : m_Faces)
        {
            for (auto& Vertex : Face.Vertices)
            {
                Vertex = Matrix * Vector4(Vertex, 0.f);
            }
        }
        BuildAABB();
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
        return uint32_t(m_Faces.size() * std::size(m_Faces[0].Vertices));
    }

    uint32_t Brush::GetIndicesCount() const
    {
        return GetVerticesCount() * 3;
    }

    //

    void Brush::BuildAABB()
    {
        Vector3 Min(std::numeric_limits<float>::max()),
            Max(std::numeric_limits<float>::lowest());

        for (auto& Face : m_Faces)
        {
            for (auto& Vertex : Face.Vertices)
            {
                Min = glm::min(Min, Vertex);
                Max = glm::max(Max, Vertex);
            }
        }

        m_AABB = {
            .Center  = (Max + Min) * 0.5f,
            .Extents = (Max - Min) * 0.5f
        };
    }

    void Brush::BuildGpuBuffer()
    {
        uint32_t VerticesCount = uint32_t(m_Faces.size() * std::size(m_Faces[0].Vertices));
        uint32_t IndicesCount  = VerticesCount * 3;
        uint32_t IndexSize     = VerticesCount < std::numeric_limits<uint16_t>::max() ? sizeof(uint16_t) : sizeof(uint32_t);

        m_Buffer = RHI::UBufferPoolHandle(
            VerticesCount * sizeof(Mdl::MeshVertex) + IndicesCount * IndexSize,
            alignof(Mdl::MeshVertex),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        auto BufferPtr = m_Buffer.AsUpload().Map() + m_Buffer.Offset;

        for (auto& Face : m_Faces)
        {
            Vector3 Normal, Tangent, Bitanget;
            // Calcuote normal
            {
                Vector3 Edge1 = Face.Vertices[1] - Face.Vertices[0];
                Vector3 Edge2 = Face.Vertices[2] - Face.Vertices[0];
                Normal        = glm::normalize(glm::cross(Edge1, Edge2));

                Vector2 DeltaUV1 = Face.UVs[1] - Face.UVs[0];
                Vector2 DeltaUV2 = Face.UVs[2] - Face.UVs[0];

                float f = 1.f / (DeltaUV1.x * DeltaUV2.y - DeltaUV2.x * DeltaUV1.y);

                Tangent.x = f * (DeltaUV2.y * Edge1.x - DeltaUV1.y * Edge2.x);
                Tangent.y = f * (DeltaUV2.y * Edge1.y - DeltaUV1.y * Edge2.y);
                Tangent.z = f * (DeltaUV2.y * Edge1.z - DeltaUV1.y * Edge2.z);
                Tangent   = glm::normalize(Tangent);

                Bitanget.x = f * (-DeltaUV2.x * Edge1.x + DeltaUV1.x * Edge2.x);
                Bitanget.y = f * (-DeltaUV2.x * Edge1.y + DeltaUV1.x * Edge2.y);
                Bitanget.z = f * (-DeltaUV2.x * Edge1.z + DeltaUV1.x * Edge2.z);
                Bitanget   = glm::normalize(Bitanget);
            }

            for (size_t i = 0; i < std::size(Face.Vertices); i++)
            {
                *std::bit_cast<Mdl::MeshVertex*>(BufferPtr) = {
                    .Position  = Face.Vertices[i],
                    .Normal    = Normal,
                    .Tangent   = Tangent,
                    .Bitangent = Bitanget,
                    .TexCoord  = Face.UVs[i],
                };
                BufferPtr += sizeof(Mdl::MeshVertex);
            }
        }

        if (IndexSize == sizeof(uint16_t))
        {
            for (uint32_t i = 0; i < VerticesCount; i++)
            {
                *std::bit_cast<uint16_t*>(BufferPtr) = i;
                BufferPtr += sizeof(uint16_t);
            }
        }
        else
        {
            for (uint32_t i = 0; i < VerticesCount; i++)
            {
                *std::bit_cast<uint32_t*>(BufferPtr) = i;
                BufferPtr += sizeof(uint32_t);
            }
        }

        m_Buffer.AsUpload().Unmap();
    }
} // namespace Neon::Scene::CSG