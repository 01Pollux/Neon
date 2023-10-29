#pragma once

#include <Mdl/Submesh.hpp>
#include <RHI/GlobalBuffer.hpp>

namespace Neon::RHI
{
    class IMaterial;
} // namespace Neon::RHI

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
        using MaterialsTable = std::vector<Ptr<RHI::IMaterial>>;
        using GPUBuffer      = std::variant<RHI::USyncGpuResource, RHI::UBufferPoolHandle>;

    public:
        Model(
            GPUBuffer&&      VertexBuffer,
            GPUBuffer&&      IndexBuffer,
            bool             SmallIndices,
            SubmeshList&&    Submeshes,
            MeshNodeList&&   Nodes,
            MaterialsTable&& Materials) noexcept :
            m_VertexBuffer(std::move(VertexBuffer)),
            m_IndexBuffer(std::move(IndexBuffer)),
            m_HasSmallIndices(SmallIndices),
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
        [[nodiscard]] RHI::GpuResourceHandle GetVertexBuffer() const noexcept
        {
            return m_VertexBuffer.index() == 0
                       ? std::get<RHI::USyncGpuResource>(m_VertexBuffer)->GetHandle()
                       : std::get<RHI::UBufferPoolHandle>(m_VertexBuffer).GetGpuHandle();
        }

        /// <summary>
        /// Get index buffer of the model.
        /// </summary>
        [[nodiscard]] RHI::GpuResourceHandle GetIndexBuffer() const noexcept
        {
            return m_IndexBuffer.index() == 0
                       ? std::get<RHI::USyncGpuResource>(m_IndexBuffer)->GetHandle()
                       : std::get<RHI::UBufferPoolHandle>(m_IndexBuffer).GetGpuHandle();
        }

        /// <summary>
        /// Check to see if index buffer's indices are 16 bits or 32.
        /// </summary>
        [[nodiscard]] bool HasSmallIndices() const noexcept
        {
            return m_HasSmallIndices;
        }

    private:
        GPUBuffer      m_VertexBuffer;
        GPUBuffer      m_IndexBuffer;
        MaterialsTable m_Materials;
        SubmeshList    m_Submeshes;
        MeshNodeList   m_Nodes;
        bool           m_HasSmallIndices;
    };
} // namespace Neon::Mdl