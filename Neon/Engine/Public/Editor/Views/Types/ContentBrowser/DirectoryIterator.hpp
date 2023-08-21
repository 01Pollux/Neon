#pragma once

#include <Asset/Metadata.hpp>

#include <Core/String.hpp>
#include <Asio/Coroutines.hpp>

#include <unordered_set>
#include <stack>
#include <filesystem>

namespace Neon::Editor::Views::CB
{
    class DirectoryIterator
    {
    public:
        using AssetMetaDataDefOpt = std::optional<Asset::AssetMetaDataDef>;
        using FileAssetMetaPair   = std::pair<std::filesystem::path, AssetMetaDataDefOpt>;

        using RootList      = std::vector<std::filesystem::path>;
        using FileAssetList = std::vector<FileAssetMetaPair>;
        using DirectoryList = std::vector<std::filesystem::path>;

        DirectoryIterator(
            const StringU8& RootPath);

        /// <summary>
        /// Visits the next the directory.
        /// </summary>
        void Visit(
            const StringU8& Directory);

        /// <summary>
        /// Goes back to the previous directory.
        /// </summary>
        void GoBack(
            size_t Steps = 1);

        /// <summary>
        /// Checks if the current directory is the root.
        /// </summary>
        [[nodiscard]] bool IsRoot() const noexcept
        {
            return m_ParentDirectories.empty();
        }

        /// <summary>
        /// Get root directories.
        /// </summary>
        [[nodiscard]] auto& GetRoots() const noexcept
        {
            return m_ParentDirectories;
        }

        /// <summary>
        /// Get files in the current directory.
        /// </summary>
        [[nodiscard]] auto& GetFiles() const noexcept
        {
            return m_Files;
        }

        /// <summary>
        /// Get directories in the current directory.
        /// </summary>
        [[nodiscard]] auto& GetDirectories() const noexcept
        {
            return m_Directories;
        }

        struct FileResult
        {
            const std::filesystem::path*   Path;
            const Asset::AssetMetaDataDef* MetaData;
            bool                           IsFile;
        };

        /// <summary>
        /// Get all files and directories in the current directory.
        /// If the result is a file, the metadata can be null, indicating that the file does not have metadata.
        /// else if the result is a directory, the metadata is always null.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<FileResult> GetAllFiles() const noexcept;

    private:
        const std::filesystem::path m_RootPath;

        RootList      m_ParentDirectories;
        FileAssetList m_Files;
        DirectoryList m_Directories;
    };
} // namespace Neon::Editor::Views::CB