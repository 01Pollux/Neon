#pragma once

#include <Math/Shapes.hpp>

namespace Neon::Geometry
{
    struct Frustum
    {
        Plane Planes[4]; // left, right, top, bottom

        Frustum(
            float Fov,
            float Aspect,
            float NearZ,
            float FarZ)
        {
            float TanFov = std::tan(Fov * 0.5f);
            float NearY  = NearZ * TanFov;
            float NearX  = NearY * Aspect;
            float FarY   = FarZ * TanFov;
            float FarX   = FarY * Aspect;

            Planes[0] = Plane(Vec::Zero<Vector3>, Vector3(1.0f, 0.0f, 1.0f), Vector3(NearX));
            Planes[1] = Plane(Vec::Zero<Vector3>, Vector3(-1.0f, 0.0f, 1.0f), Vector3(NearX));
            Planes[2] = Plane(Vec::Zero<Vector3>, Vector3(0.0f, 1.0f, 1.0f), Vector3(NearY));
            Planes[3] = Plane(Vec::Zero<Vector3>, Vector3(0.0f, -1.0f, 1.0f), Vector3(NearY));
        }
    };
} // namespace Neon::Geometry