#pragma once

#include <Asset/Asset.hpp>
#include <Asio/Coroutines.hpp>

#include <unordered_map>
#include <shared_mutex>

namespace Neon::AAsset
{
    class IAssetPackage
    {
        friend class StorageImpl;

    protected:
        using AssetCacheMap = std::unordered_map<AAsset::Handle, Ptr<IAsset>>;

        using RLock  = std::shared_lock<std::shared_mutex>;
        using RWLock = std::unique_lock<std::shared_mutex>;

    public:
        virtual ~IAssetPackage() = default;

        /// <summary>
        /// Get the assets in this package as a coroutine.
        /// </summary>
        [[nodiscard]] virtual Asio::CoGenerator<const AAsset::Handle&> GetAssets() = 0;

        /// <summary>
        /// Check if this package contains an asset.
        /// </summary>
        virtual bool ContainsAsset(
            const AAsset::Handle& AssetGuid) const = 0;

    protected:
        /// <summary>
        /// Add an asset to this package.
        /// </summary>
        virtual void AddAsset(
            Ptr<IAsset> Asset) = 0;

        /// <summary>
        /// Remove an asset from this package.
        /// </summary>
        virtual bool RemoveAsset(
            const AAsset::Handle& AssetGuid) = 0;

    protected:
        AssetCacheMap             m_Cache;
        mutable std::shared_mutex m_CacheMutex;
    };
} // namespace Neon::AAsset