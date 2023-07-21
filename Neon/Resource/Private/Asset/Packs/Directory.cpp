#include <ResourcePCH.hpp>
#include <Asset/Packs/Directory.hpp>
#include <Core/SHA256.hpp>

#include <fstream>
#include <boost/property_tree/ini_parser.hpp>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    DirectoryAssetPackage::DirectoryAssetPackage(
        const std::filesystem::path& Path)
    {
        if (Path.empty() || Path.native().starts_with(STR("..")))
        {
            NEON_ERROR_TAG("Asset", "Path '{}' cannot be empty or start with '..'", Path.string());
            return;
        }

        if (!std::filesystem::exists(Path))
        {
            NEON_TRACE_TAG("Asset", "Path '{}' does not exist, creating a new one", Path.string());
            std::filesystem::create_directories(Path);
            return;
        }

        if (!std::filesystem::is_directory(Path))
        {
            NEON_ERROR_TAG("Asset", "Trying to create a directory asset package from a file '{}'", Path.string());
            return;
        }

        NEON_TRACE_TAG("Asset", "Loading assets from directory '{}'", Path.string());

        // First load all the meta files
        for (auto& MetaFilePath : GetFiles(Path))
        {
            constexpr size_t MetaFileExtensionLength = sizeof(".namd") - 1;
            if (!MetaFilePath.native().ends_with(STR(".namd")))
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
                AssetMetaData MetaData(MetaFile);

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

                m_AssetMeta.emplace(Guid, std::move(MetaFile));
            }
        }
    }

    Asio::CoGenerator<const AAsset::Handle&> DirectoryAssetPackage::GetAssets()
    {
        RLock Lock(m_CacheMutex);
        for (auto& Guid : m_Cache | std::views::keys)
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
    }

    void DirectoryAssetPackage::AddAsset(
        Ptr<IAsset> Asset)
    {
        RWLock Lock(m_CacheMutex);
        m_Cache[Asset->GetGuid()] = std::move(Asset);
    }

    bool DirectoryAssetPackage::RemoveAsset(
        const AAsset::Handle& AssetGuid)
    {
        RWLock Lock(m_CacheMutex);
        return m_Cache.erase(AssetGuid) > 0;
    }

    Ptr<IAsset> DirectoryAssetPackage::LoadAsset(
        const AAsset::Handle& AssetGuid)
    {
        auto Iter = m_AssetMeta.find(AssetGuid);
        if (Iter == m_AssetMeta.end())
        {
            return nullptr;
        }

        AssetMetaData MetaData(Iter->second);
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
