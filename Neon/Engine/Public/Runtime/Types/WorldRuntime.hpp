#pragma once

#include <Runtime/Runtime.hpp>

namespace Neon::Runtime
{
    class EngineWorldRuntime : public EngineRuntime
    {
    public:
        EngineWorldRuntime(
            DefaultGameEngine* Engine);
    };
} // namespace Neon::Runtime