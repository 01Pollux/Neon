#pragma once

#include <Core/Neon.hpp>
#include <Resource/Asset.hpp>

namespace Neon::Asset
{
    class IAssetPack
    {
    public:
        virtual ~IAssetPack() = default;

        /// <summary>
        /// Load asset from pack file.
        /// </summary>
        virtual Ref<IAssetResource> Load(
            const AssetHandle& Handle) = 0;

        /// <summary>
        /// Remove asset from pack file.
        /// </summary>
        virtual void Remove(
            const AssetHandle& Handle) = 0;

        /// <summary>
        /// Clear the pack file's cache.
        /// </summary>
        virtual void Flush() = 0;
    };
} // namespace Neon::Asset