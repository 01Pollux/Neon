#pragma once

#include <Core/Neon.hpp>
#include <Mdl/Model.hpp>

namespace Neon::Mdl
{
    class Mesh
    {
    public:
        Mesh(
            const Ptr<Model>&       Model,
            Model::SubmeshRefList&& Submeshes) :
            m_Model(Model),
            m_Submeshes(std::move(Submeshes))
        {
        }

        /// <summary>
        /// Get model of the mesh.
        /// </summary>
        const auto& GetModel() const noexcept
        {
            return m_Model;
        }

        /// <summary>
        /// Get submeshes of the model.
        /// </summary>
        const auto& GetSubmeshes() const noexcept
        {
            return m_Submeshes;
        }

    private:
        Ptr<Model>            m_Model;
        Model::SubmeshRefList m_Submeshes;
    };
} // namespace Neon::Mdl