#pragma once

#include <Config/Engine.hpp>

namespace Neon::Config
{
    struct EditorConfig : public EngineConfig
    {
        /// <summary>
        /// The path to the project that should be loaded on startup.
        /// </summary>
        StringU8 StartupProjectPath;

        /// <summary>
        /// The name of the project that should be created on startup.
        /// </summary>
        StringU8 NewProjectName;
    };
} // namespace Neon::Config