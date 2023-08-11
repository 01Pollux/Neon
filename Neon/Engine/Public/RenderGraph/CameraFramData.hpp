#pragma once

#include <Math/Matrix.hpp>

namespace Neon::RG
{
    struct CameraFrameData
    {
        alignas(16) Matrix4x4 World;

        alignas(16) Matrix4x4 View;
        alignas(16) Matrix4x4 Projection;
        alignas(16) Matrix4x4 ViewProjection;

        alignas(16) Matrix4x4 ViewInverse;
        alignas(16) Matrix4x4 ProjectionInverse;
        alignas(16) Matrix4x4 ViewProjectionInverse;

        float EngineTime;
        float GameTime;
        float DeltaTime;
    };
} // namespace Neon::RG