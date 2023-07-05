#pragma once

#include <Resource/Manager.hpp>

namespace PakC
{
    using AssetResourcePtr = Neon::Ptr<Neon::Asset::IAssetResource>;
    using AssetPackPtr     = Neon::UPtr<Neon::Asset::IAssetPack>;

    class CustomAssetManager : public Neon::Asset::IAssetManager
    {
    public:
        CustomAssetManager();

        [[nodiscard]] Neon::Ptr<Neon::Asset::IAssetPack> OpenPack(
            const Neon::StringU8& Path) override;
    };
} // namespace PakC