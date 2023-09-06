#include <EnginePCH.hpp>
#include <Editor/Views/Types/ContentBrowser/DirectoryIterator.hpp>

#include <fstream>
#include <Log/Logger.hpp>

namespace Neon::Editor::Views::CB
{
    static void ScanDirectories(
        const std::filesystem::path&      Path,
        DirectoryIterator::FileAssetList& Files,
        DirectoryIterator::DirectoryList& Directories)
    {
        Files.clear();
        Directories.clear();

        for (const auto& Entry : std::filesystem::directory_iterator(Path))
        {
            // If the entry is a directory
            if (Entry.is_directory())
            {
                Directories.emplace_back(Entry.path());
            }
            // If the file is not a meta data file
            else if (Entry.is_regular_file() && Entry.path().extension() != Asset::AssetMetaDataDef::s_MetaFileExtensionW)
            {
                std::ifstream MetaDataFile(Entry.path().wstring() + Asset::AssetMetaDataDef::s_MetaFileExtensionW);
                if (!MetaDataFile)
                {
                    Files.emplace_back(Entry.path(), std::nullopt);
                }
                else
                {
                    Files.emplace_back(Entry.path(), MetaDataFile);
                }
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
        NEON_ASSERT(std::ranges::find_if(m_Directories, [&Directory](auto& Dir)
                                         { return Dir == Directory; }) != m_Directories.end(),
                    "Directory does not exist.");

        m_ParentDirectories.push_back(Directory);
        ScanDirectories(CurrentRoot(), m_Files, m_Directories);
    }

    void DirectoryIterator::GoBack(
        size_t Steps)
    {
        NEON_ASSERT(Steps <= m_ParentDirectories.size(), "Steps is greater than the number of parent directories.");

        for (size_t i = 0; i < Steps; ++i)
        {
            m_ParentDirectories.pop_back();
        }

        ScanDirectories(CurrentRoot(), m_Files, m_Directories);
    }

    //

    std::filesystem::path DirectoryIterator::CurrentRoot() const noexcept
    {
        return BuildPath(m_RootPath, m_ParentDirectories);
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