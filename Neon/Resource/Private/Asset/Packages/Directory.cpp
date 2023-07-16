#include <ResourcePCH.hpp>
#include <Asset/Packages/Directory.hpp>
#include <Asset/PackageDescriptor.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Handler.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>

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
        for (auto& FileInfo : Descriptor.GetPaths())
        {
            if (FileInfo.Path.empty() || FileInfo.Path.starts_with(".."))
            {
                NEON_ERROR_TAG("Asset", "Asset file '{}' is not valid", FileInfo.Path);
                return;
            }

            std::filesystem::path Path(DirPath / std::move(FileInfo.Path));
            if (!std::filesystem::exists(Path))
            {
                NEON_ERROR_TAG("Asset", "Asset file '{}' does not exist", Path.string());
                continue;
            }

            if (!std::filesystem::is_regular_file(Path))
            {
                NEON_ERROR_TAG("Asset", "Asset file '{}' is not a regular file", Path.string());
                continue;
            }

            if (m_HandleToFilePathMap.contains(FileInfo.Handle))
            {
                NEON_ERROR_TAG("Asset", "Asset file '{}' has a duplicate handle '{}'", Path.string(), FileInfo.Handle.ToString());
                continue;
            }

            m_HandleToFilePathMap.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(FileInfo.Handle),
                std::forward_as_tuple(Path.string(), std::move(FileInfo.HandlerName)));
        }
    }

    void PackageDirectory::Mount(
        Storage* AssetStorage)
    {
        for (auto& Key : m_HandleToFilePathMap | std::views::keys)
        {
            AssetStorage->Reference(this, Key);
        }
    }

    void PackageDirectory::Unmount(
        Storage* AssetStorage)
    {
        for (auto& Key : m_HandleToFilePathMap | std::views::keys)
        {
            AssetStorage->Unreference(this, Key);
        }
    }

    Asio::CoLazy<Ptr<IAsset>> PackageDirectory::Load(
        Storage*      AssetStorage,
        const Handle& ResHandle)
    {
        auto Iter = m_HandleToFilePathMap.find(ResHandle);
        if (Iter == m_HandleToFilePathMap.end())
        {
            NEON_ERROR_TAG("Asset", "Trying to load non-existing asset '{}'", ResHandle.ToString());
            co_return nullptr;
        }

        auto& HandleInfo = Iter->second;

        std::ifstream File(HandleInfo.FilePath);
        if (!File.is_open())
        {
            NEON_ERROR_TAG("Asset", "Trying to load asset '{}' with non-existing file '{}'", ResHandle.ToString(), HandleInfo.FilePath);
            co_return nullptr;
        }

        IAssetHandler* Handler = AssetStorage->GetHandler(HandleInfo.HandlerName);
        if (!Handler)
        {
            NEON_ERROR_TAG("Asset", "Trying to load asset '{}' with non-existing handler '{}'", ResHandle.ToString(), HandleInfo.HandlerName);
        }

        boost::archive::polymorphic_text_iarchive TextArchive(File);

        IO::InArchive2& Archive(TextArchive);

        Handle AssetHandle;
        Archive >> AssetHandle;

        AssetDependencyGraph Graph;

        auto Asset = Handler->Load(Archive, AssetHandle, Graph);
        for (auto& Tasks : Graph.Compile())
        {
            co_await Tasks.Resolve(AssetStorage);
        }

        co_return co_await Asset;
    }
} // namespace Neon::AAsset