#include <EnginePCH.hpp>
#include <Scene/CSG/Brush.hpp>
#include <Math/Matrix.hpp>
#include <Mdl/Submesh.hpp>

namespace Neon::Scene::CSG
{
    struct ModelInfo
    {
        void* VertexBuffer;
        void* IndexBuffer;

        RHI::UBufferPoolHandle VertexBufferHandle;
        RHI::UBufferPoolHandle IndexBufferHandle;

        ModelInfo(
            const Mdl::Mesh& Mesh)
        {
            auto& Model   = Mesh.GetModel();
            auto& Submesh = Mesh.GetData();

            std::tie(VertexBuffer, VertexBufferHandle) = Model->PeekVertexBuffer(
                Submesh.VertexOffset * sizeof(Mdl::MeshVertex),
                Submesh.VertexCount * sizeof(Mdl::MeshVertex));
            std::tie(IndexBuffer, IndexBufferHandle) = Model->PeekIndexBuffer(
                Submesh.VertexOffset * sizeof(Mdl::MeshVertex),
                Submesh.VertexCount * sizeof(Mdl::MeshVertex));
        }
    };

    void Brush::Rebuild()
    {
        ModelInfo Target(m_Target);
        ModelInfo Brush_(m_Brush);

        switch (m_Operation)
        {
        case Operation::Union:
        {
            break;
        }
        case Operation::Subtraction:
            break;
        case Operation::Intersection:
            break;
        default:
            break;
        }
    }
} // namespace Neon::Scene::CSG