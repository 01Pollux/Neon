#pragma once

#include <filesystem>

namespace Neon::FileSystem
{
    /// <summary>
    /// Show the file in explorer. this opens the file's parent directory, and highlights the file.
    /// </summary>
    void ShowFileInExplorer(
        const std::filesystem::path& Path);

    /// <summary>
    /// Open the directory in explorer.
    /// </summary>
    void OpenDirectoryInExplorer(
        const std::filesystem::path& Path);

    /// <summary>
    /// Open the file in the default program.
    /// </summary>
    void OpenExternally(
        const std::filesystem::path& Path);
} // namespace Neon::FileSystem