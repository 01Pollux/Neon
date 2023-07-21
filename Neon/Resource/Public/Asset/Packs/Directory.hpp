#pragma once

#include <Asset/Pack.hpp>
#include <Asset/Metadata.hpp>
#include <filesystem>
#include <unordered_map>

namespace Neon::AAsset
{
    class DirectoryAssetPackage : public IAssetPackage
    {
        using AssetMetaMap = std::unordered_map<Handle, boost::property_tree::ptree>;

    public:
        DirectoryAssetPackage(
            const std::filesystem::path& Path);

        [[nodiscard]] Asio::CoGenerator<const AAsset::Handle&> GetAssets() override;

        bool ContainsAsset(
            const AAsset::Handle& AssetGuid) const override;

        void Export(
            const StringU8& Path) override;

    protected:
        void AddAsset(
            Ptr<IAsset> Asset) override;

        bool RemoveAsset(
            const AAsset::Handle& AssetGuid) override;

    private:
        [[nodiscard]] Asio::CoGenerator<const std::filesystem::path&> GetFiles(
            const std::filesystem::path& Path);

    private:
        AssetMetaMap m_AssetMeta;
    };
} // namespace Neon::AAsset