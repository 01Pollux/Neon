#include <EnginePCH.hpp>
#include <Scene/CSG/Brush.hpp>
#include <Math/Matrix.hpp>

namespace Neon::Scene::CSG
{
    Brush::Brush(
        FaceList     Faces,
        MaterialList Materials) :
        m_Faces(std::move(Faces)),
        m_Materials(std::move(Materials))
    {
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
    }

    auto Brush::GetFaces() const -> const FaceList&
    {
        return m_Faces;
    }

    auto Brush::GetMaterials() const -> const MaterialList&
    {
        return m_Materials;
    }
} // namespace Neon::Scene::CSG