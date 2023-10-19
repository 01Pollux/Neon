#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>

#include <Geometry/AABB.hpp>

#include <RHI/Resource/Common.hpp>
#include <vector>

namespace Neon::Mdl
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
        Matrix4x4 Transform = Mat::Identity<Matrix4x4>;

        Geometry::AABoundingBox3D AABB;

        uint32_t VertexCount;
        uint32_t IndexCount;

        uint32_t VertexOffset;
        uint32_t IndexOffset;

        uint32_t               MaterialIndex;
        RHI::PrimitiveTopology Topology = RHI::PrimitiveTopology::Undefined;
    };

    struct MeshNode
    {
        uint32_t              Parent    = std::numeric_limits<uint32_t>::max();
        Matrix4x4             Transform = Mat::Identity<Matrix4x4>;
        StringU8              Name;
        std::vector<uint32_t> Children;
        std::vector<uint32_t> Submeshes;
    };
} // namespace Neon::Mdl