#pragma once

#include <Asset/Pack.hpp>

namespace Neon::AAsset
{
    class MemoryAssetPackage : public IAssetPackage
    {
    public:
        [[nodiscard]] Asio::CoGenerator<const AAsset::Handle&> GetAssets() override;

        bool ContainsAsset(
            const AAsset::Handle& AssetGuid) const override;

        void Export(
            const StringU8& Path) override
        {
        }

    protected:
        void AddAsset(
            Ptr<IAsset> Asset) override;

        bool RemoveAsset(
            const AAsset::Handle& AssetGuid) override;

    protected:
        Ptr<IAsset> LoadAsset(
            const AAsset::Handle& AssetGuid) override;

        bool UnloadAsset(
            const AAsset::Handle& AssetGuid) override;
    };
} // namespace Neon::AAsset