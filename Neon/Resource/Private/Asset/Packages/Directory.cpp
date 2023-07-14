#include <ResourcePCH.hpp>
#include <Asset/Packages/Directory.hpp>
#include <Asset/PackageDescriptor.hpp>
#include <Asset/Storage.hpp>

#include <IO/Archive2.hpp>
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
                NEON_ERROR_TAG("Asset", "Trying to mount a directory that is not valid");
                return;
            }

            std::filesystem::path Path(DirPath / std::move(FileInfo.Path));
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

            if (m_HandleToFilePathMap.contains(FileInfo.Handle))
            {
                NEON_ERROR_TAG("Asset", "Trying to insert an asset handle that already exists");
                continue;
            }

            m_HandleToFilePathMap.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(FileInfo.Handle),
                std::forward_as_tuple(Path.string(), std::move(FileInfo.HandlerName)));
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
        Storage*      AssetStorage,
        const Handle& Handle)
    {
        auto Iter = m_HandleToFilePathMap.find(Handle);
        if (Iter == m_HandleToFilePathMap.end())
        {
            NEON_ERROR_TAG("Asset", "Trying to load non-existing asset '{}'", Handle.ToString());
            return Ptr<IAsset>();
        }

        auto& HandleInfo = Iter->second;

        IAssetHandler* Handler = AssetStorage->GetHandler(HandleInfo.HandlerName);
        if (!Handler)
        {
            NEON_ERROR_TAG("Asset", "Trying to load asset '{}' with non-existing handler '{}'", Handle.ToString(), HandleInfo.HandlerName);
        }

        auto Graph = Handler->BuildDependencyGraph(Handle);

        return Ptr<IAsset>();
    }
} // namespace Neon::AAsset