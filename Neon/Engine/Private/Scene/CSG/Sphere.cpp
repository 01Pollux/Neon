#include <EnginePCH.hpp>
#include <Scene/CSG/Shape.hpp>
#include <RHI/Material/Shared.hpp>

namespace Neon::Scene::CSG
{
    Mdl::Mesh CreateSphere(
        float                      Radius,
        uint32_t                   RingCount,
        uint32_t                   SectorCount,
        const Ptr<RHI::IMaterial>& Material)
    {
        RingCount   = std::max(RingCount, 1u);
        SectorCount = std::max(SectorCount, 1u);

        auto ActualMaterial = Material ? Material : RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::Lit)->CreateInstance();

        //

        uint32_t VerticesCount = (RingCount + 1) * (SectorCount - 1) + 2;
        uint32_t IndicesCount  = (RingCount) * (SectorCount - 1) * 6;
        bool     Is32Bits      = VerticesCount >= std::numeric_limits<uint16_t>::max();

        RHI::UBufferPoolHandle VertexBuffer(
            VerticesCount * sizeof(Mdl::MeshVertex),
            alignof(Mdl::MeshVertex),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        RHI::UBufferPoolHandle IndexBuffer(
            IndicesCount * (Is32Bits ? sizeof(uint32_t) : sizeof(uint16_t)),
            Is32Bits ? alignof(uint32_t) : alignof(uint16_t),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        //

        auto VertexBufferPtr = VertexBuffer.AsUpload().Map<Mdl::MeshVertex>(VertexBuffer.Offset);
        auto IndexBufferU16  = IndexBuffer.AsUpload().Map<uint16_t>(IndexBuffer.Offset);
        auto IndexBufferU32  = std::bit_cast<uint32_t*>(IndexBufferU16);

        auto AppendIndex = [Is32Bits, &IndexBufferU16, &IndexBufferU32](uint32_t Index)
        {
            if (Is32Bits)
            {
                *IndexBufferU32++ = Index;
            }
            else
            {
                *IndexBufferU16++ = uint16_t(Index);
            }
        };

        //

        Mdl::Model::SubmeshList Submeshes{
            Mdl::SubMeshData{ .VertexCount = VerticesCount, .IndexCount = IndicesCount }
        };
        Mdl::Model::MeshNodeList Nodes{
            Mdl::MeshNode{ .Name = "Sphere", .Submeshes = { 0 } }
        };
        Mdl::Model::MaterialsTable Materials{ ActualMaterial };

        //

        Vector3 Min(std::numeric_limits<float>::max()),
            Max(std::numeric_limits<float>::lowest());

        //

        *VertexBufferPtr =
            Mdl::MeshVertex{
                .Position  = Vector3(0.f, Radius, 0.f),
                .Normal    = Vec::Up<Vector3>,
                .Tangent   = Vec::Right<Vector3>,
                .Bitangent = Vec::Forward<Vector3>,
                .TexCoord  = Vector2(0.f, 0.f)
            };

        Min = glm::min(Min, VertexBufferPtr->Position);
        Max = glm::max(Max, VertexBufferPtr->Position);

        ++VertexBufferPtr;

        float PhiStep   = std::numbers::pi_v<float> / RingCount;
        float ThetaStep = 2.0f * std::numbers::pi_v<float> / SectorCount;

        Mdl::MeshVertex Vtx;

        // Compute vertices for each stack ring (do not count the poles as rings).
        for (uint32_t i = 1; i <= RingCount - 1; ++i)
        {
            float Phi = i * PhiStep;

            // Vertices of ring.
            for (uint32_t j = 0; j <= SectorCount; ++j)
            {
                float Theta = j * ThetaStep;

                // spherical to cartesian
                Vtx.Position.x = Radius * sinf(Phi) * cosf(Theta);
                Vtx.Position.y = Radius * cosf(Phi);
                Vtx.Position.z = Radius * sinf(Phi) * sinf(Theta);

                // Partial derivative of P with respect to theta
                Vtx.Tangent.x = -Radius * sinf(Phi) * sinf(Theta);
                Vtx.Tangent.y = 0.0f;
                Vtx.Tangent.z = Radius * sinf(Phi) * cosf(Theta);

                Vtx.Normal  = glm::normalize(Vtx.Position);
                Vtx.Tangent = glm::normalize(Vtx.Tangent);

                Vtx.Bitangent = glm::normalize(glm::cross(Vtx.Tangent, Vtx.Normal));

                Vtx.TexCoord.x = Theta / (2.0f * std::numbers::pi_v<float>);
                Vtx.TexCoord.y = Phi / std::numbers::pi_v<float>;

                Min = glm::min(Min, Vtx.Position);
                Max = glm::max(Max, Vtx.Position);

                *VertexBufferPtr++ = Vtx;
            }
        }

        *VertexBufferPtr =
            Mdl::MeshVertex{
                .Position  = Vector3(0.f, -Radius, 0.f),
                .Normal    = Vec::Down<Vector3>,
                .Tangent   = Vec::Right<Vector3>,
                .Bitangent = Vec::Backward<Vector3>,
                .TexCoord  = Vector2(0.f, 1.f)
            };

        Min = glm::min(Min, VertexBufferPtr->Position);
        Max = glm::max(Max, VertexBufferPtr->Position);

        ++VertexBufferPtr;

        //

        Submeshes[0].AABB.Extents = (Max - Min) * 0.5f;

        //

        // Compute indices for top stack.  The top stack was written first to the vertex buffer
        // and connects the top pole to the first ring.
        //

        for (uint32_t i = 1; i <= SectorCount; ++i)
        {
            AppendIndex(0);
            AppendIndex(i + 1);
            AppendIndex(i);
        }

        //
        // Compute indices for inner stacks (not connected to poles).
        //

        // Offset the indices to the index of the first vertex in the first ring.
        // This is just skipping the top pole vertex.
        uint32_t BaseIndex       = 1;
        uint32_t RingVertexCount = SectorCount + 1;
        for (uint32_t i = 0; i < RingCount - 2; ++i)
        {
            for (uint32_t j = 0; j < SectorCount; ++j)
            {
                AppendIndex(BaseIndex + i * RingVertexCount + j);
                AppendIndex(BaseIndex + i * RingVertexCount + j + 1);
                AppendIndex(BaseIndex + (i + 1) * RingVertexCount + j);

                AppendIndex(BaseIndex + (i + 1) * RingVertexCount + j);
                AppendIndex(BaseIndex + i * RingVertexCount + j + 1);
                AppendIndex(BaseIndex + (i + 1) * RingVertexCount + j + 1);
            }
        }

        //
        // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
        // and connects the bottom pole to the bottom ring.
        //

        // South pole vertex was added last.
        uint32_t SouthPoleIndex = uint32_t(VerticesCount) - 1;

        // Offset the indices to the index of the first vertex in the last ring.
        BaseIndex = SouthPoleIndex - RingVertexCount;

        for (uint32_t i = 0; i < SectorCount; ++i)
        {
            AppendIndex(SouthPoleIndex);
            AppendIndex(BaseIndex + i);
            AppendIndex(BaseIndex + i + 1);
        }

        VertexBuffer.AsUpload().Unmap();
        IndexBuffer.AsUpload().Unmap();

        return Mdl::Mesh(
            std::make_shared<Mdl::Model>(
                std::move(VertexBuffer),
                std::move(IndexBuffer),
                !Is32Bits,
                std::move(Submeshes),
                std::move(Nodes),
                std::move(Materials)),
            0);
    }
} // namespace Neon::Scene::CSG