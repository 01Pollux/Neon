#pragma once

#include <Asset/Pack.hpp>
#include <filesystem>

namespace Neon::Asset
{
    class DirectoryAssetPackage : public IAssetPackage
    {
        using AssetMetaMap = std::unordered_map<Handle, AssetMetaDataDef>;
        using AssetPathMap = std::unordered_map<StringU8, Handle>;

    public:
        DirectoryAssetPackage(
            std::filesystem::path Path);

        [[nodiscard]] Asio::CoGenerator<const Asset::Handle&> GetAssets() override;

        bool ContainsAsset(
            const Asset::Handle& AssetGuid) const override;

    public:
        Asset::Handle FindAsset(
            const StringU8& Path) const;

        Asio::CoGenerator<Asset::Handle> FindAssets(
            const StringU8& PathRegex) const;

    public:
        std::future<void> Export() override;

        std::future<void> SaveAsset(
            Ptr<IAsset> Asset) override;

        bool RemoveAsset(
            const Asset::Handle& AssetGuid) override;

    public:
        /// <summary>
        /// Get asset's guid from a path, or returns an empty guid if not found.
        /// </summary>
        const Asset::Handle& GetGuidOfPath(
            const StringU8& Path) const;

    protected:
        Ptr<IAsset> LoadAsset(
            const Asset::Handle& AssetGuid) override;

        bool UnloadAsset(
            const Asset::Handle& AssetGuid) override;

    private:
        [[nodiscard]] Asio::CoGenerator<const std::filesystem::path&> GetFiles(
            const std::filesystem::path& Path);

    private:
        std::filesystem::path m_RootPath;
        AssetMetaMap          m_AssetMeta;
        AssetPathMap          m_AssetPath;
    };
} // namespace Neon::Asset