#pragma once

#include <Core/Neon.hpp>

namespace Neon::Runtime
{
    /// <summary>
    /// This interface is used to run the logic of the engine.
    /// It can be used to run a game, a splash screen, a closing screen, etc.
    /// </summary>
    class IEngineRuntime
    {
    public:
        IEngineRuntime() = default;

        NEON_CLASS_NO_COPYMOVE(IEngineRuntime);

        virtual ~IEngineRuntime() = default;

        /// <summary>
        /// Run the runtime.
        /// </summary>
        virtual void Run() = 0;
    };
} // namespace Neon::Runtime