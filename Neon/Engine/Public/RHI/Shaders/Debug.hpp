#pragma once

#include <RHI/Shaders/Common.hpp>

namespace Neon::RHI::Shaders
{
#ifndef NEON_DIST
    class DebugLineShader : public GlobalShader
    {
    public:
        DebugLineShader() :
            GlobalShader(DebugLineShaderGuid())
        {
        }
    };
#endif
} // namespace Neon::RHI::Shaders