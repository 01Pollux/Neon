#pragma once

#include <Core/Neon.hpp>

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
        struct Face
        {
            Vector3 Vertices[3];
            Vector2 UVs[3];
            int     MaterialIndex;

        public:
            friend class boost::serialization::access;

            template<typename _Archive>
            void serialize(
                _Archive&          Archive,
                const unsigned int Version)
            {
                Archive& Vertices;
                Archive& UVs;
                Archive& MaterialIndex;
            }
        };

        using FaceList     = std::vector<Face>;
        using MaterialList = std::vector<Ptr<RHI::IMaterial>>;

    public:
        Brush() = default;

        Brush(
            FaceList     Faces,
            MaterialList Materials);

        Brush(
            const Brush&           Brush,
            const TransformMatrix& Transform);

        /// <summary>
        /// Get aabb of the box.
        /// </summary>
        [[nodiscard]] const Geometry::AABB& GetAABB() const
        {
            return m_AABB;
        }

        /// <summary>
        /// Get faces of the brush.
        /// </summary>
        [[nodiscard]] const FaceList& GetFaces() const
        {
            return m_Faces;
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

    private:
        /// <summary>
        /// Build aabb of the brush.
        /// </summary>
        void BuildAABB();

        /// <summary>
        /// Build gpu buffer of the brush.
        /// </summary>
        void BuildGpuBuffer();

    public:
        friend class boost::serialization::access;

        template<typename _Archive>
        void serialize(
            _Archive&          Archive,
            const unsigned int Version)
        {
            Archive& m_Faces;
            // TODO: add material serialization
        }

    private:
        Geometry::AABB m_AABB;
        FaceList       m_Faces;
        MaterialList   m_Materials;

        RHI::UBufferPoolHandle m_Buffer;
    };
} // namespace Neon::Scene::CSG