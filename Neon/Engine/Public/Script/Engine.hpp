#pragma once

namespace Neon::Scripting
{
    struct ScriptConfig
    {
        int         Port    = 39391;
        const char* Version = "v4.5";
    };

    /// <summary>
    /// Initializes the scripting engine.
    /// </summary>
    void Initialize(
        const ScriptConfig& Config);

    /// <summary>
    /// Shuts down the scripting engine.
    /// </summary>
    void Shutdown();
} // namespace Neon::Scripting