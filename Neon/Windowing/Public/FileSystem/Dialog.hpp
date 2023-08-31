#pragma once

#include <Core/String.hpp>
#include <filesystem>
#include <span>

struct GLFWwindow;

namespace Neon::FileSystem
{
    struct FileDialogFilter
    {
        const wchar_t* Description;
        const wchar_t* Extension;
    };

    /// <summary>
    /// Open file dialog and return path to the requested file
    /// </summary>
    [[nodiscard]] std::vector<std::filesystem::path> OpenFile(
        GLFWwindow*                  Window,
        const String&                Title,
        const std::filesystem::path& InitialAbsoluteDir,
        std::span<FileDialogFilter>  Filters);

    /// <summary>
    /// Save file dialog and return path to the target file
    /// </summary>
    [[nodiscard]] std::filesystem::path SaveFile(
        GLFWwindow*                  Window,
        const String&                Title,
        const std::filesystem::path& InitialAbsoluteDir,
        std::span<FileDialogFilter>  Filters);
} // namespace Neon::FileSystem