#pragma once

#include <Core/Neon.hpp>

namespace Neon::Runtime
{
    class IEngineRuntime
    {
    public:
        IEngineRuntime() = default;

        NEON_CLASS_NO_COPYMOVE(IEngineRuntime);

        virtual ~IEngineRuntime() = default;
    };
} // namespace Neon::Runtime