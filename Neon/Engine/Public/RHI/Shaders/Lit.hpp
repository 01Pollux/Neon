#pragma once

#include <RHI/Shaders/Common.hpp>

namespace Neon::RHI::Shaders
{
    class LitShader : public GlobalShader
    {
    public:
        LitShader() :
            GlobalShader(LitShaderGuid())
        {
        }
    };

    class LitSpriteShader : public GlobalShader
    {
    public:
        LitSpriteShader() :
            GlobalShader(LitSpriteShaderGuid())
        {
        }
    };
} // namespace Neon::RHI::Shaders