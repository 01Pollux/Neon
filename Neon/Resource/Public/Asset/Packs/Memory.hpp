#pragma once

#include <Asset/Pack.hpp>

namespace Neon::Asset
{
    class MemoryAssetPackage : public IAssetPackage
    {
    public:
        [[nodiscard]] Asio::CoGenerator<const Asset::Handle&> GetAssets() override;

        bool ContainsAsset(
            const Asset::Handle& AssetGuid) const override;

    public:
        Asset::Handle FindAsset(
            const StringU8& Path) const override;

        Asio::CoGenerator<Asset::Handle> FindAssets(
            const StringU8& PathRegex) const override;

    public:
        std::future<void> Export() override;

        std::future<void> SaveAsset(
            Ptr<IAsset> Asset) override;

        bool RemoveAsset(
            const Asset::Handle& AssetGuid) override;

    protected:
        Ptr<IAsset> LoadAsset(
            const Asset::Handle& AssetGuid) override;

        bool UnloadAsset(
            const Asset::Handle& AssetGuid,
            bool                 Force) override;
    };
} // namespace Neon::Asset