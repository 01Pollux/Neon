#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser/DirectoryIterator.hpp>

#include <fstream>
#include <Log/Logger.hpp>

namespace Neon::Editor::Views::CB
{
    /// <summary>
    /// Builds a path from a root path and a list of parent directories.
    /// </summary>
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

    static void ScanDirectories(
        const std::filesystem::path&      RootPath,
        const std::filesystem::path&      Path,
        DirectoryIterator::FileAssetList& Files,
        DirectoryIterator::DirectoryList& Directories)
    {
        Files.clear();
        Directories.clear();

        for (const auto& Entry : std::filesystem::directory_iterator(Path))
        {
            auto RelEntry = FileSystem::ConvertToUnixPath(std::filesystem::relative(Entry.path(), RootPath));

            // If the entry is a directory
            if (Entry.is_directory())
            {
                Directories.emplace(RelEntry);
            }
            // If the file is not a meta data file
            else if (Entry.is_regular_file() && Entry.path().extension() != Asset::AssetMetaDataDef::s_MetaFileExtensionW)
            {
                std::ifstream MetaDataFile(Entry.path().wstring() + Asset::AssetMetaDataDef::s_MetaFileExtensionW);
                if (!MetaDataFile)
                {
                    Files.emplace(RelEntry, std::nullopt);
                }
                else
                {
                    Files.emplace(RelEntry, MetaDataFile);
                }
            }
        }
    }

    //

    DirectoryIterator::DirectoryIterator(
        const std::filesystem::path& RootPath) :
        m_RootPath(RootPath)
    {
        m_CurrentRoot = BuildPath(m_RootPath, m_ParentDirectories);
        ScanDirectories(m_RootPath, m_RootPath, m_Files, m_Directories);
    }

    void DirectoryIterator::Visit(
        const std::filesystem::path& Directory)
    {
        NEON_ASSERT(m_Directories.contains(Directory), "Directory does not exist.");

        m_ParentDirectories.emplace_back(Directory);
        m_CurrentRoot = BuildPath(m_RootPath, m_ParentDirectories);
        ScanDirectories(m_RootPath, CurrentRoot(), m_Files, m_Directories);
    }

    void DirectoryIterator::GoBack(
        size_t Steps)
    {
        if (Steps)
        {
            NEON_ASSERT(Steps <= m_ParentDirectories.size(), "Steps is greater than the number of parent directories.");

            m_ParentDirectories.erase(
                m_ParentDirectories.end() - Steps,
                m_ParentDirectories.end());

            m_CurrentRoot = BuildPath(m_RootPath, m_ParentDirectories);
            ScanDirectories(m_RootPath, CurrentRoot(), m_Files, m_Directories);
        }
    }

    //

    void DirectoryIterator::DeferRefresh(
        const std::filesystem::path& Path)
    {
        if (Path == CurrentRoot() ||
            Path == m_RootPath ||
            m_Directories.contains(Path) ||
            m_Files.contains(Path))
        {
            m_DeferRefresh = true;
        }
    }

    void DirectoryIterator::Update()
    {
        if (m_DeferRefresh)
        {
            m_DeferRefresh = false;
            m_CurrentRoot  = BuildPath(m_RootPath, m_ParentDirectories);
            ScanDirectories(m_RootPath, CurrentRoot(), m_Files, m_Directories);
        }
    }

    const std::filesystem::path& DirectoryIterator::CurrentRoot() const noexcept
    {
        return m_CurrentRoot;
    }

    auto DirectoryIterator::GetAllFiles() const noexcept -> Asio::CoGenerator<FileResult>
    {
        for (const auto& Directory : m_Directories)
        {
            co_yield FileResult{ &Directory, nullptr, false };
        }
        for (const auto& File : m_Files)
        {
            co_yield FileResult{ &File.first, File.second ? &*File.second : nullptr, true };
        }
    }
} // namespace Neon::Editor::Views::CB