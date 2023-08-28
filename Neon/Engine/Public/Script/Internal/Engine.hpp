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

    /// <summary>
    /// Loads the assembly.
    /// TODO: This will be replaced with a proper asset manager.
    /// </summary>
    void LoadAssembly(
        const char*     Name,
        const StringU8& Path);

    /// <summary>
    /// Unloads the assembly.
    /// </summary>
    void UnloadAssembly(
        const char* Name);
} // namespace Neon::Scripting