#pragma once

#include <Config/Script.hpp>

namespace Neon::Scripting
{
    /// <summary>
    /// Initializes the scripting engine.
    /// </summary>
    void Initialize(
        const Config::ScriptConfig& Config);

    /// <summary>
    /// Shuts down the scripting engine.
    /// </summary>
    void Shutdown();
} // namespace Neon::Scripting