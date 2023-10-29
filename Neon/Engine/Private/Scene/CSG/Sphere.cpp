#include <EnginePCH.hpp>
#include <Scene/CSG/Shape.hpp>
#include <RHI/Material/Shared.hpp>

namespace Neon::Scene::CSG
{
    // Sphere::Sphere(
    //     float                      Radius,
    //     uint32_t                   RingCount,
    //     uint32_t                   SectorCount,
    //     const Ptr<RHI::IMaterial>& Material) :
    //     m_Radius(Radius),
    //     m_RingCount(RingCount),
    //     m_SectorCount(SectorCount),
    //     m_Material(Material ? Material : RHI::SharedMaterials::Get(RHI::SharedMaterials::Type::Lit))
    //{
    //     Rebuild();
    // }

    // void Sphere::Rebuild()
    //{
    //     Mdl::MeshVertex TopVertex{
    //         .Position  = { 0.0f, +m_Radius, 0.0f },
    //         .Normal    = Vec::Up<Vector3>,
    //         .Tangent   = Vec::Right<Vector3>,
    //         .Bitangent = Vec::Forward<Vector3>,
    //         .TexCoord  = { 0.f, 0.f }
    //     };

    //    Mdl::MeshVertex BottomVertex{
    //        .Position  = { 0.0f, -m_Radius, 0.0f },
    //        .Normal    = Vec::Down<Vector3>,
    //        .Tangent   = Vec::Right<Vector3>,
    //        .Bitangent = Vec::Backward<Vector3>,
    //        .TexCoord  = { 0.f, 1.f }
    //    };

    //    // meshData.Vertices.push_back(topVertex);

    //    // float phiStep   = XM_PI / stackCount;
    //    // float thetaStep = 2.0f * XM_PI / sliceCount;

    //    //// Compute vertices for each stack ring (do not count the poles as rings).
    //    // for (uint32 i = 1; i <= stackCount - 1; ++i)
    //    //{
    //    //     float phi = i * phiStep;

    //    //    // Vertices of ring.
    //    //    for (uint32 j = 0; j <= sliceCount; ++j)
    //    //    {
    //    //        float theta = j * thetaStep;

    //    //        Vertex v;

    //    //        // spherical to cartesian
    //    //        v.Position.x = radius * sinf(phi) * cosf(theta);
    //    //        v.Position.y = radius * cosf(phi);
    //    //        v.Position.z = radius * sinf(phi) * sinf(theta);

    //    //        // Partial derivative of P with respect to theta
    //    //        v.TangentU.x = -radius * sinf(phi) * sinf(theta);
    //    //        v.TangentU.y = 0.0f;
    //    //        v.TangentU.z = +radius * sinf(phi) * cosf(theta);

    //    //        XMVECTOR T = XMLoadFloat3(&v.TangentU);
    //    //        XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

    //    //        XMVECTOR p = XMLoadFloat3(&v.Position);
    //    //        XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

    //    //        v.TexC.x = theta / XM_2PI;
    //    //        v.TexC.y = phi / XM_PI;

    //    //        meshData.Vertices.push_back(v);
    //    //    }
    //    //}

    //    // meshData.Vertices.push_back(bottomVertex);

    //    ////
    //    //// Compute indices for top stack.  The top stack was written first to the vertex buffer
    //    //// and connects the top pole to the first ring.
    //    ////

    //    // for (uint32 i = 1; i <= sliceCount; ++i)
    //    //{
    //    //     meshData.Indices32.push_back(0);
    //    //     meshData.Indices32.push_back(i + 1);
    //    //     meshData.Indices32.push_back(i);
    //    // }

    //    ////
    //    //// Compute indices for inner stacks (not connected to poles).
    //    ////

    //    //// Offset the indices to the index of the first vertex in the first ring.
    //    //// This is just skipping the top pole vertex.
    //    // uint32 baseIndex       = 1;
    //    // uint32 ringVertexCount = sliceCount + 1;
    //    // for (uint32 i = 0; i < stackCount - 2; ++i)
    //    //{
    //    //     for (uint32 j = 0; j < sliceCount; ++j)
    //    //     {
    //    //         meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j);
    //    //         meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
    //    //         meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

    //    //        meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
    //    //        meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
    //    //        meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
    //    //    }
    //    //}

    //    ////
    //    //// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
    //    //// and connects the bottom pole to the bottom ring.
    //    ////

    //    //// South pole vertex was added last.
    //    // uint32 southPoleIndex = (uint32)meshData.Vertices.size() - 1;

    //    //// Offset the indices to the index of the first vertex in the last ring.
    //    // baseIndex = southPoleIndex - ringVertexCount;

    //    // for (uint32 i = 0; i < sliceCount; ++i)
    //    //{
    //    //     meshData.Indices32.push_back(southPoleIndex);
    //    //     meshData.Indices32.push_back(baseIndex + i);
    //    //     meshData.Indices32.push_back(baseIndex + i + 1);
    //    // }

    //    // m_Brush = Brush(Faces, { m_Material }, Indices);
    //}
} // namespace Neon::Scene::CSG