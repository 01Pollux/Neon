#pragma once

#include <RHI/Shaders/Common.hpp>

namespace Neon::RHI::Shaders
{
    class GridFrustumGenShader : public GlobalShader
    {
    public:
        GridFrustumGenShader() :
            GlobalShader(GridFrustumGenShaderGuid())
        {
        }
    };
} // namespace Neon::RHI::Shaders