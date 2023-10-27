#include <EnginePCH.hpp>
#include <Scene/CSG/Shape.hpp>
#include <RHI/Material/Shared.hpp>

namespace Neon::Scene
{
    CSGBox3D::CSGBox3D() :
        CSGBox3D(
            Vec::One<Vector3>,
            RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::Lit))
    {
    }

    CSGBox3D::CSGBox3D(
        const Vector3&             Size,
        const Ptr<RHI::IMaterial>& Material) :
        m_Size(Size),
        m_Material(Material)
    {
        Rebuild();
    }

    void CSGBox3D::Rebuild()
    {
        constexpr size_t FaceCount = 12;

        CSGBrush::FaceList     Faces(FaceCount * 3);
        CSGBrush::MaterialList Materials(FaceCount);

        const Vector3 HalfSize = m_Size * 0.5f;

        constexpr Vector2 UVs[4] = {
            Vector2(0.f, 0.f),
            Vector2(1.f, 0.f),
            Vector2(1.f, 1.f),
            Vector2(0.f, 1.f)
        };

        for (size_t i = 0; i < FaceCount; ++i)
        {
            const size_t FaceIndex = i * 3;

            const bool IsFrontFace = i % 2 == 0;

            const Vector3 Normal = IsFrontFace ? Vec::Forward<Vector3> : Vec::Backward<Vector3>;

            const Vector3 Tangent = IsFrontFace ? Vec::Right<Vector3> : Vec::Left<Vector3>;

            const Vector3 Bitangent = glm::cross(Normal, Tangent);

            const Vector3 Vertices[4] = {
                HalfSize * (Normal + Tangent + Bitangent),
                HalfSize * (Normal + Tangent - Bitangent),
                HalfSize * (Normal - Tangent - Bitangent),
                HalfSize * (Normal - Tangent + Bitangent)
            };

            const CSGBrush::Face Face{
                { Vertices[0], Vertices[1], Vertices[2] },
                { UVs[0], UVs[1], UVs[2] },
                0
            };

            Faces[FaceIndex] = Face;

            const CSGBrush::Face Face2{
                { Vertices[0], Vertices[2], Vertices[3] },
                { UVs[0], UVs[2], UVs[3] },
                0
            };

            Faces[FaceIndex + 1] = Face2;
        }
        Materials[0] = m_Material;

        m_Brush = CSGBrush(std::move(Faces), std::move(Materials));
    }
} // namespace Neon::Scene