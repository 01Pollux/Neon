#pragma once

#include <Mdl/Submesh.hpp>
#include <RHI/Resource/Resource.hpp>

namespace Neon::RHI
{
        class IMaterial;
} // namespace Neon

namespace Neon::Mdl
{
    class Model
    {
    public:
        using SubmeshIndex                        = uint32_t;
        static constexpr auto InvalidSubmeshIndex = std::numeric_limits<SubmeshIndex>::max();

        using SubmeshList    = std::vector<SubMeshData>;
        using MeshNodeList   = std::vector<MeshNode>;
        using SubmeshRefList = std::vector<SubmeshIndex>;
        using GPUBuffer      = RHI::USyncGpuResource;
        using MaterialsTable = std::vector<Ptr<RHI::IMaterial>>;

    public:
        Model(
            GPUBuffer&&      VertexBuffer,
            GPUBuffer&&      IndexBuffer,
            SubmeshList&&    Submeshes,
            MeshNodeList&&   Nodes,
            MaterialsTable&& Materials) noexcept :
            m_VertexBuffer(std::move(VertexBuffer)),
            m_IndexBuffer(std::move(IndexBuffer)),
            m_Submeshes(std::move(Submeshes)),
            m_Nodes(std::move(Nodes)),
            m_Materials(std::move(Materials))
        {
        }

    public:
        /// <summary>
        /// Get root node of the model.
        /// </summary>
        [[nodiscard]] const auto& GetRootNode() const noexcept
        {
            return m_Nodes[0];
        }

        /// <summary>
        /// Get nodes of the model.
        /// </summary>
        [[nodiscard]] const auto& GetNodes() const noexcept
        {
            return m_Nodes;
        }

        /// <summary>
        /// Get nodes of the model.
        /// </summary>
        [[nodiscard]] const auto& GetNode(
            uint32_t Index) const noexcept
        {
            return m_Nodes[Index];
        }

        /// <summary>
        /// Get submeshes of the model.
        /// </summary>
        [[nodiscard]] const auto& GetSubmeshes() const noexcept
        {
            return m_Submeshes;
        }

        /// <summary>
        /// Get submeshes of the model.
        /// </summary>
        [[nodiscard]] const auto& GetSubmesh(
            uint32_t Index) const noexcept
        {
            return m_Submeshes[Index];
        }

        /// <summary>
        /// Get materials of the model.
        /// </summary>
        [[nodiscard]] const auto& GetMaterials() const noexcept
        {
            return m_Materials;
        }

        /// <summary>
        /// Get material of the model.
        /// </summary>
        [[nodiscard]] const auto& GetMaterial(
            SubmeshIndex Index) const noexcept
        {
            return m_Materials[Index];
        }

        /// <summary>
        /// Get vertex buffer of the model.
        /// </summary>
        [[nodiscard]] const auto& GetVertexBuffer() const noexcept
        {
            return m_VertexBuffer;
        }

        /// <summary>
        /// Get index buffer of the model.
        /// </summary>
        [[nodiscard]] const auto& GetIndexBuffer() const noexcept
        {
            return m_IndexBuffer;
        }

    private:
        GPUBuffer      m_VertexBuffer;
        GPUBuffer      m_IndexBuffer;
        MaterialsTable m_Materials;
        SubmeshList    m_Submeshes;
        MeshNodeList   m_Nodes;
    };
} // namespace Neon::Mdl