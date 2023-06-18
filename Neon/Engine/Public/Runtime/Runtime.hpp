#pragma once

#include <Core/Neon.hpp>

namespace Neon::Runtime
{
    class DefaultGameEngine;
    class EngineRuntime
    {
    public:
        EngineRuntime() = default;

        NEON_CLASS_NO_COPYMOVE(EngineRuntime);

        virtual ~EngineRuntime() = default;
    };
} // namespace Neon::Runtime