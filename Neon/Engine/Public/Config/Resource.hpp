#pragma once

#include <Asset/Pack.hpp>
#include <Asset/Handle.hpp>

namespace Neon::Config
{
    struct ResourceConfig
    {
        std::vector<UPtr<Asset::IAssetPackage>> AssetPackages;
        std::optional<Asset::Handle>            LoggerAssetUid;
    };
} // namespace Neon::Config