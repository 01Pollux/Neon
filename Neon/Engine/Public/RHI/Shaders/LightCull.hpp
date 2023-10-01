#pragma once

#include <RHI/Shaders/Common.hpp>

namespace Neon::RHI::Shaders
{
    class LightCullShader : public GlobalShader
    {
    public:
        LightCullShader() :
            GlobalShader(LightCullShaderGuid())
        {
        }
    };
} // namespace Neon::RHI::Shaders