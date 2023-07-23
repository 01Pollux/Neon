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

        std::future<void> Export() override
        {
            return std::async(std::launch::async, [] {});
        }

        std::future<void> SaveAsset(
            Ptr<IAsset> Asset) override;

        bool RemoveAsset(
            const Asset::Handle& AssetGuid) override;

    protected:
        Ptr<IAsset> LoadAsset(
            const Asset::Handle& AssetGuid) override;

        bool UnloadAsset(
            const Asset::Handle& AssetGuid) override;
    };
} // namespace Neon::Asset