#pragma once

#include <Asset/Pack.hpp>
#include <filesystem>

namespace Neon::AAsset
{
    class DirectoryAssetPackage : public IAssetPackage
    {
        using AssetMetaMap = std::unordered_map<Handle, AssetMetaDataDef>;

        static constexpr const char*    s_MetaFileExtension       = ".namd";
        static constexpr const wchar_t* s_MetaFileExtensionW      = L".namd";
        static constexpr size_t         s_MetaFileExtensionLength = 5;

    public:
        DirectoryAssetPackage(
            std::filesystem::path Path);

        [[nodiscard]] Asio::CoGenerator<const AAsset::Handle&> GetAssets() override;

        bool ContainsAsset(
            const AAsset::Handle& AssetGuid) const override;

        std::future<void> Export() override;

        std::future<void> SaveAsset(
            Ptr<IAsset> Asset) override;

        bool RemoveAsset(
            const AAsset::Handle& AssetGuid) override;

    protected:
        Ptr<IAsset> LoadAsset(
            const AAsset::Handle& AssetGuid) override;

        bool UnloadAsset(
            const AAsset::Handle& AssetGuid) override;

    private:
        [[nodiscard]] Asio::CoGenerator<const std::filesystem::path&> GetFiles(
            const std::filesystem::path& Path);

    private:
        std::filesystem::path m_RootPath;
        AssetMetaMap          m_AssetMeta;
    };
} // namespace Neon::AAsset