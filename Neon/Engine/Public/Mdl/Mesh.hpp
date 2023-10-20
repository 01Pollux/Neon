#pragma once

#include <Core/Neon.hpp>
#include <Mdl/Model.hpp>

namespace Neon::Mdl
{
    class Mesh
    {
    public:
        Mesh() = default;

        Mesh(
            const Ptr<Model>&   Model,
            Model::SubmeshIndex Submesh) :
            m_Model(Model),
            m_Submesh(Submesh)
        {
        }

        /// <summary>
        /// Get model of the mesh.
        /// </summary>
        [[nodiscard]] const auto& GetModel() const noexcept
        {
            return m_Model;
        }

        /// <summary>
        /// Get submesh index of the mesh.
        /// </summary>
        [[nodiscard]] uint32_t GetDataIndex() const noexcept
        {
            return m_Submesh;
        }

        /// <summary>
        /// Get submesh index of the mesh.
        /// </summary>
        [[nodiscard]] const auto& GetData() const noexcept
        {
            return m_Model->GetSubmesh(m_Submesh);
        }

        /// <summary>
        /// Get material of the mesh.
        /// </summary>
        [[nodiscard]] const auto& GetMaterial() const noexcept
        {
            return m_Model->GetMaterial(GetData().MaterialIndex);
        }

    private:
        Ptr<Model>          m_Model;
        Model::SubmeshIndex m_Submesh = Model::InvalidSubmeshIndex;
    };
} // namespace Neon::Mdl