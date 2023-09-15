#include <ResourcePCH.hpp>
#include <Private/Asset/Storage.hpp>
#include <Asset/Packs/Directory.hpp>
#include <Asset/Handler.hpp>

#include <Crypto/Sha256.hpp>
#include <queue>
#include <stack>
#include <fstream>
#include <regex>

#include <Log/Logger.hpp>

namespace Neon::Asset
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
            if (!MetafilePath.native().ends_with(AssetMetaDataDef::s_MetaFileExtensionW))
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

                if ((m_RootPath / Metadata.GetPath()) != MetafilePath)
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' has a different path than the one in meta file", MetafilePath.string());
                    continue;
                }

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

                auto AssetFile = m_RootPath / Metadata.GetAssetPath();
                File.open(AssetFile, std::ios::ate | std::ios::binary);

                if (!File.is_open())
                {
                    NEON_ERROR_TAG("Asset", "Failed to open asset file '{}'", AssetFile.string());
                    continue;
                }

                size_t FileSize = File.tellg();
                File.seekg(std::ios::beg);

                Crypto::Sha256 Hash;
                Hash.Append(File, FileSize);

                auto ExpectedHash = Metadata.GetHash();
                if (ExpectedHash.empty())
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' does not have a hash", MetafilePath.string());
                    continue;
                }
                else if (auto CurrentHash = Hash.Digest().ToString(); CurrentHash != ExpectedHash)
                {
#ifndef NEON_ASSET_MGR_DISABLE_HASH_VALIDATION
                    NEON_ERROR_TAG("Asset", "Asset file '{}' has a different hash than the one in meta file", AssetFile.string());
                    continue;
#else
                    // Try to correct the hash
                    NEON_WARNING_TAG("Asset", "Asset file '{}' has a different hash than the one in meta file", AssetFile.string());
                    Metadata.SetHash(CurrentHash);
                    Metadata.SetDirty();
#endif
                }

                m_AssetPath.emplace(Metadata.GetAssetPath().string(), Guid);
                m_AssetMeta.emplace(Guid, std::move(Metadata));
            }
        }
    }

    Asio::CoGenerator<const Asset::Handle&> DirectoryAssetPackage::GetAssets()
    {
        RLock Lock(m_CacheMutex);
        for (auto& Guid : m_AssetMeta | std::views::keys)
        {
            co_yield Guid;
        }
    }

    bool DirectoryAssetPackage::ContainsAsset(
        const Asset::Handle& AssetGuid) const
    {
        RLock Lock(m_CacheMutex);
        return m_AssetMeta.contains(AssetGuid);
    }

    Asset::Handle DirectoryAssetPackage::FindAsset(
        const StringU8& Path) const
    {
        RLock Lock(m_CacheMutex);
        auto  Iter = m_AssetPath.find(Path);
        return Iter != m_AssetPath.end() ? Iter->second : Asset::Handle::Null;
    }

    Asio::CoGenerator<Asset::Handle> DirectoryAssetPackage::FindAssets(
        const StringU8& PathRegex) const
    {
        std::regex Regex(PathRegex);

        RLock Lock(m_CacheMutex);
        for (auto& [Path, Handle] : m_AssetPath)
        {
            if (std::regex_match(Path, Regex))
            {
                co_yield Asset::Handle{ Handle };
            }
        }
    }

    std::future<void> DirectoryAssetPackage::Export()
    {
        auto ExportTask = [this]
        {
            // Export all dirty assets
            {
                RWLock Lock(m_CacheMutex);
                for (auto& [Guid, Metadata] : m_AssetMeta)
                {
                    if (!Metadata.IsDirty())
                    {
                        continue;
                    }

                    ExportMeta(Metadata);
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
                        m_AssetPath.emplace(CurrentAsset->GetPath(), AssetGuid);
                        Iter->second.SetPath(StringUtils::Format("{}{}", CurrentAsset->GetPath(), AssetMetaDataDef::s_MetaFileExtension));
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

                auto AssetPath = m_RootPath / Metadata->GetAssetPath();
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
                std::vector<uint8_t> Buffer(FileSize);
                AssetFile.read(reinterpret_cast<char*>(Buffer.data()), FileSize);

                Crypto::Sha256 Hash;
                Hash.Append(Buffer.data(), FileSize);
                Metadata->SetLoaderId(HandlerId);
                Metadata->SetHash(Hash.Digest().ToString());

                CurrentAsset->MarkDirty(false);
                Metadata->SetDirty();
            }
        };

        return StorageImpl::Get()->GetThreadPool().enqueue(std::move(SaveAssetTask));
    }

    bool DirectoryAssetPackage::RemoveAsset(
        const Asset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);

        auto Iter = m_AssetMeta.find(AssetGuid);
        if (Iter == m_AssetMeta.end())
        {
            return false;
        }

        m_Cache.erase(AssetGuid);
        m_AssetPath.erase(Iter->second.GetPath());
        m_AssetMeta.erase(Iter);
        return true;
    }

    const Asset::Handle& DirectoryAssetPackage::GetGuidOfPath(
        const StringU8& Path) const
    {
        RLock Lock(m_CacheMutex);
        auto  Iter = m_AssetPath.find(Path);
        return Iter != m_AssetPath.end() ? Iter->second : Asset::Handle::Null;
    }

    Ptr<IAsset> DirectoryAssetPackage::LoadAsset(
        const Asset::Handle& AssetGuid)
    {
        auto LoadFromCache =
            [this](const Asset::Handle& AssetGuid) -> Ptr<IAsset>
        {
            RLock Lock(m_CacheMutex);
            if (auto Iter = m_Cache.find(AssetGuid); Iter != m_Cache.end())
            {
                return Iter->second;
            }
            return nullptr;
        };

        // The reason for checking if the asset is already loaded in the cache
        // is because we later will only load the asset if it's not already loaded
        if (auto CacheAsset = LoadFromCache(AssetGuid))
        {
            return CacheAsset;
        }

        std::stack<Handle> ToLoad;
        ToLoad.push(AssetGuid);

        Asset::DependencyReader DepReader;

        while (!ToLoad.empty())
        {
            auto& CurrentGuid = ToLoad.top();

            // Check if asset is already loaded in cache
            // If not, check if it exists in the package
            AssetMetaDataDef* Metadata = nullptr;
            {
                RLock Lock(m_CacheMutex);
                auto  Iter = m_AssetMeta.find(CurrentGuid);
                if (Iter == m_AssetMeta.end())
                {
                    NEON_ERROR_TAG("Asset", "Loading '{}' that depends on '{}' failed: Asset does not exist", AssetGuid.ToString(), CurrentGuid.ToString());
                    return nullptr;
                }

                Metadata = &Iter->second;
            }

            // If we need to load dependencies first, skip this asset and load the dependencies
            bool NeedsToDependenciesFirst = false;

            // Insert the assets that should be loaded first
            {
                RLock Lock(m_CacheMutex);
                for (auto& DepGuid : Metadata->GetDependencies())
                {
                    if (auto CacheAsset = LoadFromCache(DepGuid))
                    {
                        DepReader.Link(DepGuid, CacheAsset);
                    }
                    else
                    {
                        ToLoad.push(std::move(DepGuid));
                        NeedsToDependenciesFirst = true;
                    }
                }
            }
            if (NeedsToDependenciesFirst)
            {
                continue;
            }

            auto AssetPath = m_RootPath / Metadata->GetAssetPath();
            if (!std::filesystem::exists(AssetPath))
            {
                NEON_ERROR_TAG("Asset", "Loading '{}' -- Asset file '{}' of GUID '{}' does not exist", AssetGuid.ToString(), AssetPath.string(), CurrentGuid.ToString());
                return nullptr;
            }

            //
            // Read asset file
            //

            IAssetHandler* Handler = Storage::GetHandler(Metadata->GetLoaderId());
            if (!Handler)
            {
                NEON_ERROR_TAG("Asset", "Loading '{}' -- Failed to get handler for asset '{}'", AssetGuid.ToString(), CurrentGuid.ToString());
                return nullptr;
            }

            std::ifstream AssetFile(AssetPath, std::ios::in | std::ios::binary);
            if (!AssetFile.is_open())
            {
                NEON_ERROR_TAG("Asset", "Loading '{}' -- Failed to open asset file '{}'", AssetGuid.ToString(), AssetPath.string());
                return nullptr;
            }

            auto Asset = Handler->Load(AssetFile, DepReader, CurrentGuid, AssetPath.string(), Metadata->GetLoaderData());
            if (!Asset)
            {
                NEON_ERROR_TAG("Asset", "Loading '{}' -- Failed to load asset '{}'", AssetGuid.ToString(), CurrentGuid.ToString());
                return nullptr;
            }

            Asset->MarkDirty(false);
            DepReader.Link(CurrentGuid, Asset);

            ToLoad.pop();

            {
                RWLock Lock(m_CacheMutex);
                m_Cache.emplace(CurrentGuid, Asset);
            }
        }

        return LoadFromCache(AssetGuid);
    }

    bool DirectoryAssetPackage::UnloadAsset(
        const Asset::Handle& AssetGuid,
        bool                 Force)
    {
        RWLock Lock(m_CacheMutex);

        auto Iter = m_Cache.find(AssetGuid);
        if (Iter == m_Cache.end())
        {
            m_Cache.erase(AssetGuid);
        }

        if (!Force && Iter->second.use_count() == 1)
        {
            m_Cache.erase(Iter);
        }
        return true;
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

    void DirectoryAssetPackage::ExportMeta(
        AssetMetaDataDef& Meta)
    {
        std::filesystem::path AssetPath = m_RootPath / Meta.GetPath();
        std::filesystem::create_directories(AssetPath.parent_path());

        std::ofstream Metafile(AssetPath, std::ios::out | std::ios::trunc);
        Meta.Export(Metafile);
        Meta.SetDirty(false);
    }
} // namespace Neon::Asset
