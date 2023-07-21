#include <ResourcePCH.hpp>
#include <Private/Asset/Storage.hpp>
#include <Asset/Packs/Directory.hpp>
#include <Asset/Handler.hpp>

#include <Core/SHA256.hpp>
#include <fstream>
#include <boost/property_tree/ini_parser.hpp>

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
        for (auto& MetaFilePath : GetFiles(m_RootPath))
        {
            constexpr size_t MetaFileExtensionLength = sizeof(".nam") - 1;
            if (!MetaFilePath.native().ends_with(STR(".nam")))
            {
                continue;
            }

            std::ifstream File(MetaFilePath);
            if (!File.is_open())
            {
                NEON_ERROR_TAG("Asset", "Failed to open meta file '{}'", MetaFilePath.string());
                continue;
            }

            {
                boost::property_tree::ptree MetaFile;
                boost::property_tree::read_ini(File, MetaFile);
                AssetMetaDataDef MetaData(MetaFile);

                File.close();

                auto Guid = MetaData.GetGuid();
                if (Guid == Handle::Null)
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' does not have a Guid", MetaFilePath.string());
                    continue;
                }
                if (m_AssetMeta.contains(Guid))
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' has a Guid that is already in use", MetaFilePath.string());
                    continue;
                }

                auto AssetFile = std::filesystem::path(MetaFilePath).replace_extension("");
                File.open(std::move(AssetFile), std::ios::end);

                if (!File.is_open())
                {
                    NEON_ERROR_TAG("Asset", "Failed to open asset file '{}'", AssetFile.string());
                    continue;
                }

                size_t FileSize = File.tellg();
                File.seekg(std::ios::beg);

                SHA256 Hash;
                Hash.Append(File, FileSize);

                auto ExpectedHash = MetaData.GetHash();
                if (ExpectedHash.empty())
                {
                    NEON_ERROR_TAG("Asset", "Meta file '{}' does not have a hash", MetaFilePath.string());
                    continue;
                }
                else if (Hash.Digest().ToString() != ExpectedHash)
                {
                    NEON_ERROR_TAG("Asset", "Asset file '{}' has a different hash than the one in meta file", AssetFile.string());
                    continue;
                }

                m_AssetMeta.emplace(Guid, std::move(MetaData));
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
        return m_Cache.contains(AssetGuid);
    }

    void DirectoryAssetPackage::Export(
        const StringU8& Path)
    {
        for (auto& [Guid, Asset] : m_AssetMeta)
        {
        }
    }

    std::future<void> DirectoryAssetPackage::AddAsset(
        Ptr<IAsset>     Asset,
        const StringU8& Path)
    {
        if (Path.empty() || Path.starts_with(".."))
        {
            NEON_ERROR_TAG("Asset", "Path '{}' cannot be empty or start with '..'", Path);
            return {};
        }

        return StorageImpl::Get()->GetThreadPool().enqueue(
            [Asset = std::move(Asset),
             Path,
             this]
            {
                auto CheckForDuplicate = [&]()
                {
                    if (m_AssetMeta.contains(Asset->GetGuid()))
                    {
                        NEON_ERROR_TAG("Asset", "Asset with Guid '{}' already exists", Asset->GetGuid().ToString());
                        return true;
                    }

                    return false;
                };

                {
                    RLock Lock(m_CacheMutex);
                    if (!CheckForDuplicate())
                    {
                        return;
                    }
                }

                auto& Guid = Asset->GetGuid();

                AssetMetaDataDef MetaData;
                MetaData.SetGuid(Guid);

                IAssetHandler* Handler = Storage::GetHandler(Asset);
                if (Handler == nullptr)
                {
                    NEON_ERROR_TAG("Asset", "Failed to get handler for asset '{}'", Guid.ToString());
                    return;
                }

                auto AssetPath  = m_RootPath / Path;
                auto ParentPath = AssetPath.parent_path();
                if (!std::filesystem::exists(ParentPath))
                {
                    std::filesystem::create_directories(ParentPath);
                }

                std::ofstream AssetFile(AssetPath, std::ios::trunc | std::ios::binary);
                if (!AssetFile.is_open())
                {
                    NEON_ERROR_TAG("Asset", "Failed to open asset file '{}'", AssetPath.string());
                    return;
                }

                Handler->Save(AssetFile, Asset, MetaData.GetLoaderData());

                RWLock Lock(m_CacheMutex);
                if (CheckForDuplicate())
                {
                    m_Cache[Guid]     = std::move(Asset);
                    m_AssetMeta[Guid] = std::move(MetaData);
                }
            });
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
        {
            RLock Lock(m_CacheMutex);
            if (auto Iter = m_Cache.find(AssetGuid); Iter != m_Cache.end())
            {
                return Iter->second;
            }
        }

        auto Iter = m_AssetMeta.find(AssetGuid);
        if (Iter == m_AssetMeta.end())
        {
            return nullptr;
        }

        return nullptr;
    }

    bool DirectoryAssetPackage::UnloadAsset(
        const AAsset::Handle& AssetGuid)
    {
        return false;
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
