#pragma once

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
        using RootList     = std::vector<std::filesystem::path>;
        using FileSet      = std::unordered_set<std::filesystem::path>;
        using DirectorySet = std::unordered_set<std::filesystem::path>;

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

        /// <summary>
        /// Get all files and directories in the current directory.
        /// The second value in the pair is true if the entry is a file.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<
            std::pair<const std::filesystem::path*, bool>>
        GetAllFiles() const noexcept;

    private:
        const std::filesystem::path m_RootPath;

        RootList     m_ParentDirectories;
        FileSet      m_Files;
        DirectorySet m_Directories;
    };
} // namespace Neon::Editor::Views::CB