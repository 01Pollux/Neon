#pragma once

#include <RHI/Shaders/Common.hpp>

namespace Neon::RHI::Shaders
{
    class CopyToTextureShader : public GlobalShader
    {
    public:
        CopyToTextureShader() :
            GlobalShader(CopyToTextureShaderGuid())
        {
        }
    };
} // namespace Neon::RHI::Shaders