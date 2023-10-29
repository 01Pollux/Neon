#include <EnginePCH.hpp>
#include <Scene/CSG/Shape.hpp>
#include <RHI/Material/Shared.hpp>

namespace Neon::Scene::CSG
{
    Mdl::Mesh CreateCube(
        Vector3                    Size,
        const Ptr<RHI::IMaterial>& Material)
    {
        Size = glm::max(Size, Vector3(0.001f));

        const Vector3 HalfSize = Size * 0.5f;

        float w2 = HalfSize.x;
        float h2 = HalfSize.y;
        float d2 = HalfSize.z;

        auto ActualMaterial = Material ? Material : RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::Lit)->CreateInstance();

        //

        constexpr size_t FaceCount     = 6;
        constexpr size_t VerticesCount = FaceCount * 4;
        constexpr size_t IndicesCount  = FaceCount * 6;

        RHI::UBufferPoolHandle VertexBuffer(
            VerticesCount * sizeof(Mdl::MeshVertex),
            alignof(Mdl::MeshVertex),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        RHI::UBufferPoolHandle IndexBuffer(
            IndicesCount * sizeof(uint16_t),
            alignof(uint16_t),
            RHI::IGlobalBufferPool::BufferType::ReadWriteGPUR);

        //

        auto VertexBufferPtr = VertexBuffer.AsUpload().Map<Mdl::MeshVertex>(VertexBuffer.Offset);
        auto IndexBufferPtr  = IndexBuffer.AsUpload().Map<uint16_t>(IndexBuffer.Offset);

        //

        Mdl::Model::SubmeshList Submeshes{
            Mdl::SubMeshData{ .AABB = { .Extents = HalfSize }, .VertexCount = VerticesCount, .IndexCount = IndicesCount }
        };
        Mdl::Model::MeshNodeList Nodes{
            Mdl::MeshNode{ .Name = "Cube", .Submeshes = { 0 } }
        };
        Mdl::Model::MaterialsTable Materials{ ActualMaterial };

        //

        constexpr uint8_t State0110[]{ 0, 1, 1, 0 };

        // Fill in the front face vertex data.
        for (int i = 0; i < 4; i++)
        {
            *VertexBufferPtr++ =
                Mdl::MeshVertex{
                    .Position  = { -w2 + (i / 2) * Size.x, -h2 + State0110[i] * Size.y, -d2 },
                    .Normal    = Vec::Backward<Vector3>,
                    .Tangent   = Vec::Right<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f }
                };
        }

        // Fill in the back face vertex data.
        for (int i = 0; i < 4; i++)
        {
            *VertexBufferPtr++ =
                Mdl::MeshVertex{
                    .Position  = { -w2 + State0110[i] * Size.x, -h2 + (i / 2) * Size.y, d2 },
                    .Normal    = Vec::Forward<Vector3>,
                    .Tangent   = Vec::Left<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (!State0110[i]) * 1.f, !(i / 2) }
                };
        }

        // Fill in the top face vertex data.
        for (int i = 0; i < 4; i++)
        {
            *VertexBufferPtr++ =
                Mdl::MeshVertex{
                    .Position  = { -w2 + (i / 2) * Size.x, h2, -d2 + State0110[i] * Size.z },
                    .Normal    = Vec::Up<Vector3>,
                    .Tangent   = Vec::Right<Vector3>,
                    .Bitangent = Vec::Forward<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f }
                };
        }

        // Fill in the bottom face vertex data.
        for (int i = 0; i < 4; i++)
        {
            *VertexBufferPtr++ =
                Mdl::MeshVertex{
                    .Position  = { -w2 + State0110[i] * Size.x, -h2, -d2 + (i / 2) * Size.z },
                    .Normal    = Vec::Down<Vector3>,
                    .Tangent   = Vec::Left<Vector3>,
                    .Bitangent = Vec::Forward<Vector3>,
                    .TexCoord  = { (!State0110[i]) * 1.f, !(i / 2) }
                };
        }

        // Fill in the left face vertex data.
        for (int i = 0; i < 4; i++)
        {
            *VertexBufferPtr++ =
                Mdl::MeshVertex{
                    .Position  = { -w2, -h2 + State0110[i] * Size.y, d2 - (i / 2) * Size.z },
                    .Normal    = Vec::Left<Vector3>,
                    .Tangent   = Vec::Backward<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f }
                };
        }

        // Fill in the right face vertex data.
        for (int i = 0; i < 4; i++)
        {
            *VertexBufferPtr++ =
                Mdl::MeshVertex{
                    .Position  = { w2, -h2 + State0110[i] * Size.y, -d2 + (i / 2) * Size.z },
                    .Normal    = Vec::Right<Vector3>,
                    .Tangent   = Vec::Forward<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f }
                };
        }

        for (uint16_t j = 0; j < 6; ++j)
        {
            IndexBufferPtr[j * 6]     = j * 4;
            IndexBufferPtr[j * 6 + 1] = j * 4 + 1;
            IndexBufferPtr[j * 6 + 2] = j * 4 + 2;
            IndexBufferPtr[j * 6 + 3] = j * 4;
            IndexBufferPtr[j * 6 + 4] = j * 4 + 2;
            IndexBufferPtr[j * 6 + 5] = j * 4 + 3;
        }

        VertexBuffer.AsUpload().Unmap();
        IndexBuffer.AsUpload().Unmap();

        return Mdl::Mesh(
            std::make_shared<Mdl::Model>(
                std::move(VertexBuffer),
                std::move(IndexBuffer),
                true,
                std::move(Submeshes),
                std::move(Nodes),
                std::move(Materials)),
            0);
    }
} // namespace Neon::Scene::CSG