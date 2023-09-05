#include <EnginePCH.hpp>
#include <Renderer/Mesh/Mesh.hpp>

namespace Neon::Renderer
{
    StaticMesh::StaticMesh(
        const Ptr<Model>&       Model,
        Model::SubmeshRefList&& Submeshes) :
        m_Model(Model),
        m_Submeshes(std::move(Submeshes))
    {
        if (m_Submeshes.empty())
        {
            m_Submeshes.resize(m_Model->GetSubmeshes().size());
            std::iota(m_Submeshes.begin(), m_Submeshes.end(), 0);
        }
    }
} // namespace Neon::Renderer
