#pragma once

#include <Resource/Manager.hpp>

namespace PakC
{
    using AssetResourcePtr = Neon::Ptr<Neon::Asset::IAssetResource>;
    using AssetPackPtr     = Neon::UPtr<Neon::Asset::IAssetPack>;

    class CustomResourceManager : public Neon::Asset::IResourceManager
    {
    public:
        CustomResourceManager();

        [[nodiscard]] Neon::UPtr<Neon::Asset::IAssetPack> OpenPack(
            const Neon::StringU8& Path) override;
    };
} // namespace PakC