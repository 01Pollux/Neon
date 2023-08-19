#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser/DirectoryIterator.hpp>

#include <Log/Logger.hpp>

namespace Neon::Editor::Views::CB
{
    static void ScanDirectories(
        const std::filesystem::path&     Path,
        DirectoryIterator::FileSet&      Files,
        DirectoryIterator::DirectorySet& Directories)
    {
        Files.clear();
        Directories.clear();

        for (const auto& Entry : std::filesystem::directory_iterator(Path))
        {
            if (Entry.is_directory())
            {
                Directories.insert(Entry.path().stem());
            }
            else if (Entry.is_regular_file())
            {
                Files.insert(Entry.path().stem().filename());
            }
        }
    }

    [[nodiscard]] static std::filesystem::path BuildPath(
        const std::filesystem::path&       RootPath,
        const DirectoryIterator::RootList& ParentDirectories)
    {
        auto Path = RootPath;

        for (const auto& Directory : ParentDirectories)
        {
            Path /= Directory;
        }

        return Path;
    }

    //

    DirectoryIterator::DirectoryIterator(
        const StringU8& RootPath) :
        m_RootPath(RootPath)
    {
        ScanDirectories(m_RootPath, m_Files, m_Directories);
    }

    void DirectoryIterator::Visit(
        const StringU8& Directory)
    {
        NEON_ASSERT(m_Directories.contains(Directory), "Directory does not exist.");

        m_ParentDirectories.push_back(Directory);
        ScanDirectories(BuildPath(m_RootPath, m_ParentDirectories), m_Files, m_Directories);
    }

    void DirectoryIterator::GoBack(
        size_t Steps)
    {
        NEON_ASSERT(Steps <= m_ParentDirectories.size(), "Steps is greater than the number of parent directories.");

        for (size_t i = 0; i < Steps; ++i)
        {
            m_ParentDirectories.pop_back();
        }

        ScanDirectories(BuildPath(m_RootPath, m_ParentDirectories), m_Files, m_Directories);
    }

    Asio::CoGenerator<std::pair<const std::filesystem::path*, bool>> DirectoryIterator::GetAllFiles() const noexcept
    {
        for (const auto& Directory : m_Directories)
        {
            co_yield { &Directory, false };
        }
        for (const auto& File : m_Files)
        {
            co_yield { &File, true };
        }
    }
} // namespace Neon::Editor::Views::CB