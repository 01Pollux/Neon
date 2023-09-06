#pragma once

#include <Core/Neon.hpp>
#include <Math/AABB.hpp>
#include <Math/Matrix.hpp>
#include <RHI/Resource/Resource.hpp>
#include <vector>

namespace Neon
{
    namespace RHI
    {
        class IBuffer;
    } // namespace RHI
    namespace Renderer
    {
        class IMaterial;
    } // namespace Renderer
} // namespace Neon

namespace Neon::Renderer
{
    struct MeshVertex
    {
        Vector3 Position;
        Vector3 Normal;
        Vector3 Tangent;
        Vector3 Bitangent;
        Vector2 TexCoord;
    };

    struct SubMeshData
    {
        Matrix4x4       Transform = Mat::Identity<Matrix4x4>;
        AABoundingBox3D AABB;

        uint32_t VertexCount;
        uint32_t IndexCount;

        uint32_t VertexOffset;
        uint32_t IndexOffset;

        uint32_t MaterialIndex;
    };

    //

    struct MeshNode
    {
        uint32_t              Parent = std::numeric_limits<uint32_t>::max();
        std::vector<uint32_t> Children;
        std::vector<uint32_t> Submeshes;
        Matrix4x4             Transform = Mat::Identity<Matrix4x4>;
        StringU8              Name;
    };

    //

    class Model
    {
    public:
        using SubmeshList    = std::vector<SubMeshData>;
        using MeshNodeList   = std::vector<MeshNode>;
        using SubmeshRefList = std::vector<uint32_t>;
        using GPUBuffer      = UPtr<RHI::IBuffer>;
        using MaterialsTable = std::vector<Ptr<IMaterial>>;

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
        /// Get submeshes of the model.
        /// </summary>
        const auto& GetSubmeshes() const noexcept
        {
            return m_Submeshes;
        }

        /// <summary>
        /// Get materials of the model.
        /// </summary>
        const auto& GetMaterials() const noexcept
        {
            return m_Materials;
        }

        /// <summary>
        /// Get root node of the model.
        /// </summary>
        const auto& GetRootNode() const noexcept
        {
            return m_Nodes[0];
        }

        /// <summary>
        /// Get nodes of the model.
        /// </summary>
        const auto& GetNodes() const noexcept
        {
            return m_Nodes;
        }

        /// <summary>
        /// Get nodes of the model.
        /// </summary>
        const auto& GetNode(
            uint32_t Index) const noexcept
        {
            return m_Nodes[Index];
        }

        /// <summary>
        /// Get vertex buffer of the model.
        /// </summary>
        const auto& GetVertexBuffer() const noexcept
        {
            return m_VertexBuffer;
        }

        /// <summary>
        /// Get index buffer of the model.
        /// </summary>
        const auto& GetIndexBuffer() const noexcept
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

    //

    class StaticMesh
    {
    public:
        StaticMesh(
            const Ptr<Model>&       Model,
            Model::SubmeshRefList&& Submeshes);

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
} // namespace Neon::Renderer