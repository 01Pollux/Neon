#include <ResourcePCH.hpp>
#include <Asset/Packages/Directory.hpp>
#include <Asset/PackageDescriptor.hpp>
#include <Asset/Storage.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    PackageDirectory::PackageDirectory(
        const std::filesystem::path& DirPath)
    {
        if (DirPath.empty() || DirPath.native().starts_with(STR("..")))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not valid");
            return;
        }

        if (!std::filesystem::exists(DirPath))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that does not exist");
            return;
        }

        if (!std::filesystem::is_directory(DirPath))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not a directory");
            return;
        }

        //

        PackageDescriptor Descriptor(std::ifstream(DirPath / "Descriptor.ini"));
        for (auto& [Key, StrPath] : Descriptor.GetPaths())
        {
            if (StrPath.empty() || StrPath.starts_with(".."))
            {
                NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not valid");
                return;
            }

            std::filesystem::path Path(DirPath / std::move(StrPath));
            if (!std::filesystem::exists(Path))
            {
                NEON_ERROR_TAG("Asset", "Trying to mount to a path that does not exist");
                continue;
            }

            if (!std::filesystem::is_regular_file(Path))
            {
                NEON_ERROR_TAG("Asset", "Trying to mount to a path that is not a file");
                continue;
            }

            if (m_HandleToFilePathMap.contains(Key))
            {
                NEON_ERROR_TAG("Asset", "Trying to insert an asset handle that already exists");
                continue;
            }

            m_HandleToFilePathMap.emplace(Key, Path.string());
        }
    }

    void PackageDirectory::Mount(
        Storage* Storage)
    {
        for (auto& Key : m_HandleToFilePathMap | std::views::keys)
        {
            Storage->Reference(this, Key);
        }
    }

    void PackageDirectory::Unmount(
        Storage* Storage)
    {
        for (auto& Key : m_HandleToFilePathMap | std::views::keys)
        {
            Storage->Unreference(this, Key);
        }
    }

    Ptr<IAsset> PackageDirectory::Load(
        const Handle& Handle)
    {
        auto Iter = m_HandleToFilePathMap.find(Handle);
        if (Iter == m_HandleToFilePathMap.end())
        {
            return Ptr<IAsset>();
        }

        std::ifstream File(Iter->second);
        if (!File.is_open())
        {
            NEON_ERROR_TAG("Asset", "Failed to open file: '{}'", Iter->second);
            return Ptr<IAsset>();
        }

        std::stringstream Stream;
        Stream << File.rdbuf();
        printf("%s\n", Stream.str().c_str());

        return Ptr<IAsset>();
    }
} // namespace Neon::AAsset