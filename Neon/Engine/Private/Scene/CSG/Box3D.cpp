#include <EnginePCH.hpp>
#include <Scene/CSG/Shape.hpp>
#include <RHI/Material/Shared.hpp>

namespace Neon::Scene::CSG
{
    Box3D::Box3D() :
        Box3D(
            Vec::One<Vector3>,
            RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::Lit))
    {
    }

    Box3D::Box3D(
        const Vector3&             Size,
        const Ptr<RHI::IMaterial>& Material) :
        m_Size(Size),
        m_Material(Material)
    {
        Rebuild();
    }

    void Box3D::Rebuild()
    {
        constexpr size_t FaceCount = 6;

        const Vector3 HalfSize = m_Size * 0.5f;

        float w2 = HalfSize.x;
        float h2 = HalfSize.y;
        float d2 = HalfSize.z;

        // TODO TexCoord
        Brush::FaceList Faces;
        Faces.reserve(6);

        Brush::Face Face;

        constexpr uint8_t State0110[]{ 0, 1, 1, 0 };

        // Fill in the front face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Face.Vertices[i] =
                Mdl::MeshVertex{
                    .Position  = { -w2 + (i / 2) * m_Size.x, -h2 + State0110[i] * m_Size.y, -d2 },
                    .Normal    = Vec::Backward<Vector3>,
                    .Tangent   = Vec::Right<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i % 2), (i / 2) }
                };
        }

        Faces.emplace_back(Face);

        // Fill in the back face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Face.Vertices[i] =
                Mdl::MeshVertex{
                    .Position  = { -w2 + State0110[i] * m_Size.x, -h2 + (i / 2) * m_Size.y, d2 },
                    .Normal    = Vec::Forward<Vector3>,
                    .Tangent   = Vec::Left<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i % 2), (i / 2) }
                };
        }
        Faces.emplace_back(Face);

        // Fill in the top face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Face.Vertices[i] =
                Mdl::MeshVertex{
                    .Position  = { -w2 + (i / 2) * m_Size.x, h2, -d2 + State0110[i] * m_Size.z },
                    .Normal    = Vec::Up<Vector3>,
                    .Tangent   = Vec::Right<Vector3>,
                    .Bitangent = Vec::Forward<Vector3>,
                    .TexCoord  = { (i % 2), (i / 2) }
                };
        }
        Faces.emplace_back(Face);

        // Fill in the bottom face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Face.Vertices[i] =
                Mdl::MeshVertex{
                    .Position  = { -w2 + State0110[i] * m_Size.x, -h2, -d2 + (i / 2) * m_Size.z },
                    .Normal    = Vec::Down<Vector3>,
                    .Tangent   = Vec::Left<Vector3>,
                    .Bitangent = Vec::Forward<Vector3>,
                    .TexCoord  = { (i % 2), (i / 2) }
                };
        }
        Faces.emplace_back(Face);

        // Fill in the left face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Face.Vertices[i] =
                Mdl::MeshVertex{
                    .Position  = { -w2, -h2 + State0110[i] * m_Size.y, d2 - (i / 2) * m_Size.z },
                    .Normal    = Vec::Left<Vector3>,
                    .Tangent   = Vec::Backward<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i % 2), (i / 2) }
                };
        }
        Faces.emplace_back(Face);

        // Fill in the right face vertex data.
        for (int i = 0; i < 4; i++)
        {
            Face.Vertices[i] =
                Mdl::MeshVertex{
                    .Position  = { w2, -h2 + State0110[i] * m_Size.y, -d2 + (i / 2) * m_Size.z },
                    .Normal    = Vec::Right<Vector3>,
                    .Tangent   = Vec::Forward<Vector3>,
                    .Bitangent = Vec::Up<Vector3>,
                    .TexCoord  = { (i % 2), (i / 2) }
                };
        }
        Faces.emplace_back(Face);

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

        m_Brush = Brush(Faces, { m_Material }, Indices);
    }
} // namespace Neon::Scene::CSG