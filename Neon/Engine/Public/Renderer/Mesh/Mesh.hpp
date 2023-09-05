#pragma once

#include <Core/Neon.hpp>
#include <Math/Vector.hpp>
#include <Math/Matrix.hpp>

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

    class SubMeshData
    {
    public:
        SubMeshData(
            const Matrix4x4& Transform,
            uint32_t         VertexCount,
            uint32_t         IndexCount,
            uint32_t         VertexOffset,
            uint32_t         IndexOffset,
            uint32_t         MaterialIndex) :
            m_Transform(Transform),
            m_VertexCount(VertexCount),
            m_IndexCount(IndexCount),
            m_VertexOffset(VertexOffset),
            m_IndexOffset(IndexOffset),
            m_MaterialIndex(MaterialIndex)
        {
        }

    private:
        Matrix4x4 m_Transform;

        uint32_t m_VertexCount;
        uint32_t m_IndexCount;

        uint32_t m_VertexOffset;
        uint32_t m_IndexOffset;

        uint32_t m_MaterialIndex;
    };

    class Model
    {
        friend class Asset::ModelAsset::Handler;

    public:
        using SubmeshList    = std::vector<SubMeshData>;
        using SubmeshRefList = std::vector<uint32_t>;
        using GPUBuffer      = UPtr<RHI::IBuffer>;
        using MaterialsTable = std::vector<Ptr<IMaterial>>;

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

    private:
        GPUBuffer      m_VertexBuffer;
        GPUBuffer      m_IndexBuffer;
        MaterialsTable m_Materials;
        SubmeshList    m_Submeshes;
    };

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