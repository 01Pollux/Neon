#include <EnginePCH.hpp>
#include <Scene/CSG/Shape.hpp>
#include <RHI/Material/Shared.hpp>

namespace Neon::Scene::CSG
{
    Box3D::Box3D(
        const Vector3&             Size,
        const Ptr<RHI::IMaterial>& Material) :
        m_Size(Size),
        m_Material(Material ? Material : RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::Lit))
    {
    }

    void Box3D::Rebuild()
    {
        constexpr size_t FaceCount = 6;

        const Vector3 HalfSize = m_Size * 0.5f;

        float w2 = HalfSize.x;
        float h2 = HalfSize.y;
        float d2 = HalfSize.z;

        Brush::VertexList Vertices;
        Vertices.reserve(FaceCount * 4);

        constexpr uint8_t State0110[]{ 0, 1, 1, 0 };

        // Fill in the front face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Vertices.emplace_back(
                Mdl::MeshVertex{
                    .Position  = { -w2 + (i / 2) * m_Size.x, -h2 + State0110[i] * m_Size.y, -d2 },
                    .Normal    = Vec::Backward<Vector3>,
                    .Tangent   = Vec::Right<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f } });
        }

        // Fill in the back face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Vertices.emplace_back(
                Mdl::MeshVertex{
                    .Position  = { -w2 + State0110[i] * m_Size.x, -h2 + (i / 2) * m_Size.y, d2 },
                    .Normal    = Vec::Forward<Vector3>,
                    .Tangent   = Vec::Left<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (!State0110[i]) * 1.f, !(i / 2) } });
        }

        // Fill in the top face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Vertices.emplace_back(
                Mdl::MeshVertex{
                    .Position  = { -w2 + (i / 2) * m_Size.x, h2, -d2 + State0110[i] * m_Size.z },
                    .Normal    = Vec::Up<Vector3>,
                    .Tangent   = Vec::Right<Vector3>,
                    .Bitangent = Vec::Forward<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f } });
        }

        // Fill in the bottom face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Vertices.emplace_back(
                Mdl::MeshVertex{
                    .Position  = { -w2 + State0110[i] * m_Size.x, -h2, -d2 + (i / 2) * m_Size.z },
                    .Normal    = Vec::Down<Vector3>,
                    .Tangent   = Vec::Left<Vector3>,
                    .Bitangent = Vec::Forward<Vector3>,
                    .TexCoord  = { (!State0110[i]) * 1.f, !(i / 2) } });
        }

        // Fill in the left face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Vertices.emplace_back(
                Mdl::MeshVertex{
                    .Position  = { -w2, -h2 + State0110[i] * m_Size.y, d2 - (i / 2) * m_Size.z },
                    .Normal    = Vec::Left<Vector3>,
                    .Tangent   = Vec::Backward<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f } });
        }

        // Fill in the right face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Vertices.emplace_back(
                Mdl::MeshVertex{
                    .Position  = { w2, -h2 + State0110[i] * m_Size.y, -d2 + (i / 2) * m_Size.z },
                    .Normal    = Vec::Right<Vector3>,
                    .Tangent   = Vec::Forward<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i / 2), (!State0110[i]) * 1.f } });
        }

        uint16_t Indices[36]{};
        for (uint16_t j = 0; j < 6; ++j)
        {
            Indices[j * 6]     = j * 4;
            Indices[j * 6 + 1] = j * 4 + 1;
            Indices[j * 6 + 2] = j * 4 + 2;
            Indices[j * 6 + 3] = j * 4;
            Indices[j * 6 + 4] = j * 4 + 2;
            Indices[j * 6 + 5] = j * 4 + 3;
        }

        m_Brush = Brush(Vertices, { m_Material }, Indices);
    }
} // namespace Neon::Scene::CSG