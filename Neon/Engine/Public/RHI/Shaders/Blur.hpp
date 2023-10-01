#pragma once

#include <RHI/Shaders/Common.hpp>

namespace Neon::RHI::Shaders
{
    class BlurShader : public GlobalShader
    {
    public:
        BlurShader() :
            GlobalShader(BlurShaderGuid())
        {
        }
    };
} // namespace Neon::RHI