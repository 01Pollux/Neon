#include <ResourcePCH.hpp>
#include <Private/Asset/Storage.hpp>
#include <Asset/Packs/Directory.hpp>
#include <Asset/Handler.hpp>

#include <Core/SHA256.hpp>
#include <queue>
#include <fstream>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    DirectoryAssetPackage::DirectoryAssetPackage(
        std::filesystem::path Path) :
        m_RootPath(std::move(Path))
    {
        if (m_RootPath.empty() || m_RootPath.native().starts_with(STR("..")))
        {
            NEON_ERROR_TAG("Asset", "Path '{}' cannot be empty or start with '..'", m_RootPath.string());
            return;
        }

        if (!std::filesystem::exists(m_RootPath))
        {
            NEON_TRACE_TAG("Asset", "Path '{}' does not exist, creating a new one", m_RootPath.string());
            std::filesystem::create_directories(m_RootPath);
            return;
        }

        if (!std::filesystem::is_directory(m_RootPath))
        {
            NEON_ERROR_TAG("Asset", "Trying to create a directory asset package from a file '{}'", m_RootPath.string());
            return;
        }

        NEON_TRACE_TAG("Asset", "Loading assets from directory '{}'", m_RootPath.string());

        // First load all the meta files
        for (auto& MetafilePath : GetFiles(m_RootPath))
        {
            if (!MetafilePath.native().ends_with(s_MetaFileExtensionW))
            {
                continue;
            }

            std::ifstream File(MetafilePath);
            if (!File.is_open())
            {
                NEON_ERROR_TAG("Asset", "Failed to open meta file '{}'", MetafilePath.string());
                continue;
            }

            {
                AssetMetaDataDef Metadata(File);
                File.close();

                auto Guid = Metadata.GetGuid();
                if (Guid == Handle::Null)
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' does not have a Guid", MetafilePath.string());
                    continue;
                }
                if (m_AssetMeta.contains(Guid))
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' has a Guid that is already in use", MetafilePath.string());
                    continue;
                }

                auto AssetFile = std::filesystem::path(MetafilePath).replace_extension("");
                File.open(AssetFile, std::ios::ate | std::ios::binary);

                if (!File.is_open())
                {
                    NEON_ERROR_TAG("Asset", "Failed to open asset file '{}'", AssetFile.string());
                    continue;
                }

                size_t FileSize = File.tellg();
                File.seekg(std::ios::beg);

                SHA256 Hash;
                Hash.Append(File, FileSize);

                auto ExpectedHash = Metadata.GetHash();
                if (ExpectedHash.empty())
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' does not have a hash", MetafilePath.string());
                    continue;
                }
                else if (Hash.Digest().ToString() != ExpectedHash)
                {
                    NEON_ERROR_TAG("Asset", "Asset file '{}' has a different hash than the one in meta file", AssetFile.string());
                    continue;
                }

                m_AssetMeta.emplace(Guid, std::move(Metadata));
            }
        }
    }

    Asio::CoGenerator<const AAsset::Handle&> DirectoryAssetPackage::GetAssets()
    {
        RLock Lock(m_CacheMutex);
        for (auto& Guid : m_AssetMeta | std::views::keys)
        {
            co_yield Guid;
        }
    }

    bool DirectoryAssetPackage::ContainsAsset(
        const AAsset::Handle& AssetGuid) const
    {
        RLock Lock(m_CacheMutex);
        return m_AssetMeta.contains(AssetGuid);
    }

    std::future<void> DirectoryAssetPackage::Export()
    {
        auto ExportTask = [this]
        {
            // Export all dirty assets
            {
                RLock Lock(m_CacheMutex);
                for (auto& [Guid, Metadata] : m_AssetMeta)
                {
                    if (!Metadata.IsDirty())
                    {
                        continue;
                    }

                    std::filesystem::path AssetPath = Metadata.GetPath();
                    std::filesystem::create_directories(AssetPath.parent_path());

                    std::ofstream Metafile(AssetPath, std::ios::out | std::ios::trunc);
                    Metadata.Export(Metafile);
                }
            }
            // Unset dirty flag
            {
                RWLock Lock(m_CacheMutex);
                for (auto& Metadata : m_AssetMeta | std::views::values)
                {
                    Metadata.SetDirty(false);
                }
            }
        };

        return StorageImpl::Get()->GetThreadPool().enqueue(std::move(ExportTask));
    }

    std::future<void> DirectoryAssetPackage::SaveAsset(
        Ptr<IAsset> FirstAsset)
    {
        auto SaveAssetTask =
            [this,
             FirstAsset]
        {
            std::queue<Ptr<IAsset>> ToSave;
            ToSave.emplace(FirstAsset);

            while (!ToSave.empty())
            {
                auto CurrentAsset = std::move(ToSave.front());
                ToSave.pop();

                auto& AssetGuid = CurrentAsset->GetGuid();

                // Check if asset already exists in the package
                // else add it to the package
                AssetMetaDataDef* Metadata = nullptr;
                {
                    RWLock Lock(m_CacheMutex);
                    auto   Iter = m_AssetMeta.find(AssetGuid);
                    if (Iter == m_AssetMeta.end())
                    {
                        Iter = m_AssetMeta.emplace(AssetGuid, AssetMetaDataDef(AssetGuid, CurrentAsset->GetPath())).first;
                        Iter->second.SetPath(StringUtils::Format("{}/{}{}", m_RootPath.string(), CurrentAsset->GetPath(), s_MetaFileExtension));
                    }
                    Metadata = &Iter->second;
                }

                //
                // Write asset file
                //

                size_t         HandlerId;
                IAssetHandler* Handler = Storage::GetHandler(CurrentAsset, &HandlerId);
                if (!Handler)
                {
                    NEON_ERROR_TAG("Asset", "Failed to get handler for asset '{}'", AssetGuid.ToString());
                    continue;
                }

                auto AssetPath = Metadata->GetAssetPath();
                std::filesystem::create_directories(AssetPath.parent_path());

                std::fstream AssetFile(AssetPath, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
                if (!AssetFile.is_open())
                {
                    NEON_ERROR_TAG("Asset", "Failed to open asset file '{}'", AssetPath.string());
                    continue;
                }

                DependencyWriter DepWriter;
                Handler->Save(AssetFile, DepWriter, CurrentAsset, Metadata->GetLoaderData());

                // Write dependencies to metadata
                {
                    std::list<StringU8> DepsInsertInMetadata;

                    auto& Dependencies = DepWriter.GetDependencies();
                    for (auto& Dep : DepWriter.GetDependencies())
                    {
                        if (Dep->IsDirty())
                        {
                            ToSave.push(Dep);
                        }
                        DepsInsertInMetadata.emplace_back(Dep->GetGuid().ToString());
                    }
                    Dependencies.clear();
                    Metadata->SetDependencies(std::move(DepsInsertInMetadata));
                }

                //
                // Write file's hash
                //

                size_t FileSize = AssetFile.tellp();
                AssetFile.seekg(std::ios::beg);

                SHA256 Hash;
                Hash.Append(AssetFile, FileSize);
                Metadata->SetLoaderId(HandlerId);
                Metadata->SetHash(Hash.Digest().ToString());

                CurrentAsset->MarkDirty(false);
                Metadata->SetDirty();
            }
        };

        return StorageImpl::Get()->GetThreadPool().enqueue(std::move(SaveAssetTask));
    }

    bool DirectoryAssetPackage::RemoveAsset(
        const AAsset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);
        return m_Cache.erase(AssetGuid) > 0 && m_AssetMeta.erase(AssetGuid) > 0;
    }

    Ptr<IAsset> DirectoryAssetPackage::LoadAsset(
        const AAsset::Handle& AssetGuid)
    {
        AssetMetaDataDef* Metadata = nullptr;
        // Check if asset is already loaded in cache
        // If not, check if it exists in the package
        {
            RLock Lock(m_CacheMutex);
            if (auto Iter = m_Cache.find(AssetGuid); Iter != m_Cache.end())
            {
                return Iter->second;
            }

            auto Iter = m_AssetMeta.find(AssetGuid);
            if (Iter == m_AssetMeta.end())
            {
                return nullptr;
            }

            Metadata = &Iter->second;
        }

        auto AssetPath = Metadata->GetAssetPath();
        if (!std::filesystem::exists(AssetPath))
        {
            NEON_ERROR_TAG("Asset", "Asset file '{}' of GUID '{}' does not exist", AssetPath.string(), AssetGuid.ToString());
            return nullptr;
        }

        //
        // Read asset file
        //

        IAssetHandler* Handler = Storage::GetHandler(Metadata->GetLoaderId());
        if (!Handler)
        {
            NEON_ERROR_TAG("Asset", "Failed to get handler for asset '{}'", AssetGuid.ToString());
            return nullptr;
        }

        std::ifstream AssetFile(AssetPath, std::ios::in | std::ios::binary);
        if (!AssetFile.is_open())
        {
            NEON_ERROR_TAG("Asset", "Failed to open asset file '{}'", AssetPath.string());
            return nullptr;
        }

        auto Asset = Handler->Load(AssetFile, AssetGuid, AssetPath.string(), Metadata->GetLoaderData());
        if (!Asset)
        {
            NEON_ERROR_TAG("Asset", "Failed to load asset '{}'", AssetGuid.ToString());
            return nullptr;
        }

        Asset->MarkDirty(false);
        return Asset;
    }

    bool DirectoryAssetPackage::UnloadAsset(
        const AAsset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);
        return m_Cache.erase(AssetGuid) > 0;
    }

    //

    Asio::CoGenerator<const std::filesystem::path&> DirectoryAssetPackage::GetFiles(
        const std::filesystem::path& Path)
    {
        for (auto& Entry : std::filesystem::recursive_directory_iterator(Path))
        {
            if (Entry.is_regular_file())
            {
                co_yield Entry.path();
            }
        }
    }
} // namespace Neon::AAsset
