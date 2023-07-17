#include <ResourcePCH.hpp>
#include <Asset/Packages/Directory.hpp>
#include <Asset/PackageDescriptor.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Handler.hpp>
#include <Asset/Asset.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all_ready.hpp>

#include <Log/Logger.hpp>

//

namespace views  = std::views;
namespace ranges = std::ranges;

//

namespace Neon::AAsset
{
    PackageDirectory::PackageDirectory(
        const std::filesystem::path& DirPath) :
        IPackage(DirPath.string())
    {
        if (DirPath.empty() || DirPath.native().starts_with(STR("..")))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not valid");
            return;
        }

        // Empty package not an error, so we create the directories if they don't exist
        if (!std::filesystem::exists(DirPath))
        {
            std::filesystem::create_directories(DirPath);
            return;
        }

        if (!std::filesystem::is_directory(DirPath))
        {
            NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not a directory");
            return;
        }

        //

        std::ifstream     DescriptorFile(DirPath / "Descriptor.ini");
        PackageDescriptor Descriptor(DescriptorFile);
        DescriptorFile.close();

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

    void PackageDirectory::AddAsset(
        const AssetAddDesc& Desc,
        const Ptr<IAsset>&  Asset)
    {
#ifndef NEON_DIST
        if (!Asset)
        {
            NEON_WARNING_TAG("Asset", "Trying to add a null asset");
            return;
        }
#endif
        std::scoped_lock AssetLock(m_PackageMutex);
        if (m_HandleToFilePathMap.contains(Asset->GetGuid()))
        {
            NEON_WARNING_TAG("Asset", "Trying to add an asset with a duplicate handle '{}'", Asset->GetGuid().ToString());
            return;
        }

        m_HandleToFilePathMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(Asset->GetGuid()),
            std::forward_as_tuple(Desc.Path, Desc.HandlerName));
        m_AssetCache.emplace(Asset->GetGuid(), Asset);
    }

    void PackageDirectory::RemoveAsset(
        const Handle& AssetHandle)
    {
        std::scoped_lock AssetLock(m_PackageMutex);
        m_HandleToFilePathMap.erase(AssetHandle);
        m_AssetCache.erase(AssetHandle);
    }

    //

    Asio::CoLazy<Ptr<IAsset>> PackageDirectory::Load(
        Storage*      AssetStorage,
        const Handle& ResHandle)
    {
        // Asset was already loaded, so we don't need to load it again
        auto AssetIter = m_AssetCache.find(ResHandle);
        if (AssetIter != m_AssetCache.end())
        {
            co_return AssetIter->second;
        }

        auto Iter = m_HandleToFilePathMap.find(ResHandle);
        if (Iter == m_HandleToFilePathMap.end())
        {
            NEON_ERROR_TAG("Asset", "Trying to load non-existing asset '{}'", ResHandle.ToString());
            co_return nullptr;
        }

        auto& Info = Iter->second;

        std::ifstream File(Info.FilePath, std::ios::binary);
        if (!File.is_open())
        {
            NEON_ERROR_TAG("Asset", "Trying to load asset '{}' with non-existing file '{}'", ResHandle.ToString(), Info.FilePath);
            co_return nullptr;
        }

        IAssetHandler* Handler = AssetStorage->GetHandler(Info.HandlerName);
        if (!Handler)
        {
            NEON_ERROR_TAG("Asset", "Trying to load asset '{}' with non-existing handler '{}'", ResHandle.ToString(), Info.HandlerName);
        }

        boost::archive::polymorphic_text_iarchive TextArchive(File);

        IO::InArchive2& Archive(TextArchive);

        Handle AssetHandle;
        Archive >> AssetHandle;

        AssetDependencyGraph Graph;

        auto AssetTask = (cppcoro::when_all_ready(
            [&]() -> Asio::CoLazy<Ptr<IAsset>>
            {
                co_return co_await Handler->Load(Archive, AssetHandle, Graph);
            }(),
            [&]() -> Asio::CoLazy<>
            {
                for (auto& Tasks : Graph.Compile())
                {
                    co_await Tasks.Load(this, AssetStorage);
                }
            }()));

        auto Asset = std::get<0>(co_await AssetTask).result();
        m_AssetCache.emplace(AssetHandle, Asset);
        co_return Asset;
    }

    //

    void PackageDirectory::Flush(
        Storage* AssetStorage)
    {
        if (m_AssetCache.empty())
        {
            return;
        }

        std::ofstream DescriptorFile(StringUtils::Format("{}/Descriptor.ini", m_PackagePath), std::ios::trunc);
        if (!DescriptorFile.is_open())
        {
            NEON_ERROR_TAG("Asset", "Failed to open file '{}/Descriptor.ini' for saving package descriptor", m_PackagePath);
            return;
        }

        PackageDescriptor Descriptor;
        for (auto& [AssetHandle, Asset] : m_AssetCache)
        {
            auto& Info       = m_HandleToFilePathMap.at(AssetHandle);
            auto  Path       = std::filesystem::path(StringUtils::Format("{}/{}", m_PackagePath, Info.FilePath));
            auto  ParentPath = Path.parent_path();

            // create path if it doesn't exist
            if (!std::filesystem::exists(ParentPath))
            {
                std::filesystem::create_directories(ParentPath);
            }

            std::ofstream AssetFile(Path, std::ios::trunc | std::ios::binary);
            if (!AssetFile.is_open())
            {
                NEON_ERROR_TAG("Asset", "Failed to open file '{}' for saving asset '{}'", Path.string(), AssetHandle.ToString());
                continue;
            }

            IAssetHandler* Handler = AssetStorage->GetHandler(Info.HandlerName);
            if (!Handler)
            {
                NEON_ERROR_TAG("Asset", "Trying to save asset '{}' with non-existing handler '{}'", AssetHandle.ToString(), Info.HandlerName);
                continue;
            }

            boost::archive::polymorphic_text_oarchive TextArchive(AssetFile);

            IO::OutArchive2& Archive(TextArchive);

            Archive << AssetHandle;
            Handler->Save(Archive, Asset);

            Descriptor.Append(AssetHandle, Info.HandlerName, Info.FilePath);
        }
        Descriptor.Save(DescriptorFile);
    }

    std::vector<Handle> PackageDirectory::GetAssets() const
    {
        return m_HandleToFilePathMap | std::views::keys | ranges::to<std::vector>();
    }
} // namespace Neon::AAsset