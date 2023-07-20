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

    protected:
        void AddAsset(
            Ptr<IAsset> Asset) override;

        bool RemoveAsset(
            const AAsset::Handle& AssetGuid) override;
    };
} // namespace Neon::AAsset