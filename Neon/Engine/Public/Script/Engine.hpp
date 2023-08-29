#pragma once

#include <Config/Script.hpp>
#include <Script/Handle.hpp>
#include <Script/Internal/Assembly.hpp>

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

    //

    /// <summary>
    /// Creates a script object.
    /// </summary>
    GCHandle CreateScriptObject(
        const char* AssemblyName,
        const char* TypeName,
        void**      Parameters,
        uint32_t    ParameterCount);

    /// <summary>
    /// Creates a script object.
    /// </summary>
    const CS::Class* GetClass(
        const char* AssemblyName,
        const char* TypeName);

    //

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