#pragma once

#include <Core/Neon.hpp>

#include <Mdl/Submesh.hpp>
#include <Math/Transform.hpp>
#include <Geometry/AABB.hpp>
#include <RHI/GlobalBuffer.hpp>

#include <vector>

namespace Neon::RHI
{
    class IMaterial;
} // namespace Neon::RHI

namespace Neon::Scene::CSG
{
    class Brush
    {
    public:
        using VertexList   = std::vector<Mdl::MeshVertex>;
        using MaterialList = std::vector<Ptr<RHI::IMaterial>>;

    public:
        Brush() = default;

        Brush(
            const VertexList& Vertices,
            MaterialList      Materials,
            const void*       Indices,
            size_t            IndicesCount,
            bool              Is16BitsIndex);

        Brush(
            const VertexList&         Vertices,
            MaterialList              Materials,
            const std::span<uint16_t> Indices) :
            Brush(Vertices, std::move(Materials), Indices.data(), Indices.size(), true)
        {
        }

        Brush(
            const VertexList&         Vertices,
            MaterialList              Materials,
            const std::span<uint32_t> Indices) :
            Brush(Vertices, std::move(Materials), Indices.data(), Indices.size(), false)
        {
        }

        /// <summary>
        /// Get aabb of the box.
        /// </summary>
        [[nodiscard]] const Geometry::AABB& GetAABB() const
        {
            return m_AABB;
        }

        /// <summary>
        /// Get materials of the brush.
        /// </summary>
        [[nodiscard]] const MaterialList& GetMaterials() const
        {
            return m_Materials;
        }

    public:
        /// <summary>
        /// Get vertex buffer of the brush.
        /// </summary>
        [[nodiscard]] RHI::GpuResourceHandle GetVertexBuffer() const;

        /// <summary>
        /// Get index buffer of the brush.
        /// </summary>
        [[nodiscard]] RHI::GpuResourceHandle GetIndexBuffer() const;

        /// <summary>
        /// Get vertices count of the brush.
        /// </summary>
        [[nodiscard]] uint32_t GetVerticesCount() const;

        /// <summary>
        /// Get indices count of the brush.
        /// </summary>
        [[nodiscard]] uint32_t GetIndicesCount() const;

        /// <summary>
        /// Check to see if index buffer is 16 bits size.
        /// </summary>
        [[nodiscard]] bool Is16BitsIndex() const;

    private:
        /// <summary>
        /// Build aabb of the brush.
        /// </summary>
        void BuildAABB(
            const VertexList& Vertices);

        /// <summary>
        /// Build gpu buffer of the brush.
        /// </summary>
        void BuildGpuBuffer(
            const VertexList& Vertices,
            const void*       Indices,
            size_t            IndicesCount);

    public:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive&          Archive,
            const unsigned int Version)
        {
            // TODO: add material serialization
        }

    private:
        Geometry::AABB m_AABB;
        MaterialList   m_Materials;

        RHI::UBufferPoolHandle m_Buffer;
        uint32_t               m_VerticesCount = 0;
        uint32_t               m_IndicesCount  = 0;
        bool                   m_Is16BitsIndex = false;
    };
} // namespace Neon::Scene::CSG