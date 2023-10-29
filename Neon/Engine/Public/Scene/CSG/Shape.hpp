#pragma once

#include <Scene/CSG/Brush.hpp>
#include <boost/serialization/vector.hpp>

namespace boost::serialization
{
    class access;
} // namespace boost::serialization

namespace Neon::Scene::CSG
{
    /// <summary>
    /// Create a cube mesh.
    /// </summary>
    [[nodiscard]] Mdl::Mesh CreateCube(
        const Vector3&             Size     = Vec::One<Vector3>,
        const Ptr<RHI::IMaterial>& Material = nullptr);

    ///// <summary>
    ///// Create a sphere mesh.
    ///// </summary>
    //[[nodiscard]] Mdl::Mesh CreateSphere(
    //    float                      Radius      = 0.5f,
    //    uint32_t                   RingCount   = 16,
    //    uint32_t                   SectorCount = 16,
    //    const Ptr<RHI::IMaterial>& Material    = nullptr);
} // namespace Neon::Scene::CSG