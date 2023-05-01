#pragma once

#include <Core/Neon.hpp>
#include <Resource/Asset.hpp>

namespace Neon::Asset
{
    class IAssetPack
    {
    public:
        using AssetResourceHandlerMap = std::map<uint32_t, UPtr<IAssetResourceHandler>>;

        virtual ~IAssetPack() = default;

        /// <summary>
        /// Load asset from pack file.
        /// </summary>
        virtual Ref<IAssetResource> Load(
            const AssetResourceHandlerMap& Handlers,
            const AssetHandle&             Handle) = 0;

    protected:
        /// <summary>
        /// Remove resource's original data from the cache
        /// </summary>
        virtual void UnrefAsset(
            const AssetHandle& Handle) = 0;
    };
} // namespace Neon::Asset