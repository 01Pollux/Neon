#pragma once

#include <Asset/Pack.hpp>
#include <Asset/Handle.hpp>
#include <vector>

namespace Neon::Config
{
    struct ResourceConfig
    {
        std::vector<UPtr<Asset::IAssetPackage>> AssetPackages;

        std::optional<Asset::Handle> LoggerAssetUid = Asset::Handle::FromString("896dc3ed-263d-4a0e-9f16-b127ecd87619");
    };
} // namespace Neon::Config