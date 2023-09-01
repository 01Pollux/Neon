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

    struct FileDialog
    {
        GLFWwindow* Window = nullptr;
        String      Title;

        /// <summary>
        /// Initial absolute path to the file or folder
        /// </summary>
        std::filesystem::path       InitialDir;
        std::span<FileDialogFilter> Filters;

        /// <summary>
        /// Open or save file dialog and return path to the requested file
        /// </summary>
        [[nodiscard]] std::filesystem::path SaveFile() const
        {
            auto Paths = OpenFileDialog(DialogType::Save);
            return Paths.empty() ? std::filesystem::path() : std::move(Paths[0]);
        }

        /// <summary>
        /// Open or save file dialog and return path to the requested file(s)
        /// </summary>
        [[nodiscard]] std::vector<std::filesystem::path> OpenFiles() const
        {
            return OpenFileDialog(DialogType::Open);
        }

        /// <summary>
        /// Open or save file dialog and return path to the requested file(s)
        /// </summary>
        [[nodiscard]] std::filesystem::path OpenFile() const
        {
            auto Paths = OpenFileDialog(DialogType::Open);
            return Paths.empty() ? std::filesystem::path() : std::move(Paths[0]);
        }

        /// <summary>
        /// Open folder dialog and return path to the requested folder(s)
        /// </summary>
        [[nodiscard]] std::vector<std::filesystem::path> OpenFolder() const
        {
            return OpenFileDialog(DialogType::Folder);
        }

    private:
        enum class DialogType : uint8_t
        {
            Save,
            Open,
            Folder
        };

        /// <summary>
        /// Open or save file dialog
        /// </summary>
        std::vector<std::filesystem::path> OpenFileDialog(
            DialogType Type) const;
    };
} // namespace Neon::FileSystem