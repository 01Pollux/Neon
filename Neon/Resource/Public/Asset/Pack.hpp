#pragma once

#include <Asset/Asset.hpp>
#include <Asset/Metadata.hpp>
#include <Asio/Coroutines.hpp>

#include <unordered_map>
#include <shared_mutex>
#include <future>

namespace Neon::Asset
{
    class IAssetPackage
    {
        friend class StorageImpl;
        friend class ManagerImpl;

    protected:
        using AssetCacheMap = std::unordered_map<Asset::Handle, Ptr<IAsset>>;

        using RLock  = std::shared_lock<std::shared_mutex>;
        using RWLock = std::unique_lock<std::shared_mutex>;

    public:
        virtual ~IAssetPackage() = default;

    public:
        /// <summary>
        /// Get the assets in this package as a coroutine.
        /// </summary>
        [[nodiscard]] virtual Asio::CoGenerator<const Asset::Handle&> GetAssets() = 0;

        /// <summary>
        /// Check if this package contains an asset.
        /// </summary>
        virtual bool ContainsAsset(
            const Asset::Handle& AssetGuid) const = 0;

    public:
        /// <summary>
        /// Finds an asset by path.
        /// </summary>
        [[nodiscard]] virtual Asset::Handle FindAsset(
            const StringU8& Path) const = 0;

        /// <summary>
        /// Finds assets by path as regex.
        /// </summary>
        [[nodiscard]] virtual Asio::CoGenerator<Asset::Handle> FindAssets(
            const StringU8& PathRegex) const = 0;

    public:
        /// <summary>
        /// Export this package to the filesystem.
        /// </summary>
        virtual std::future<void> Export() = 0;

        /// <summary>
        /// Add an asset to this package.
        /// </summary>
        virtual std::future<void> SaveAsset(
            Ptr<IAsset> Asset) = 0;

        /// <summary>
        /// Remove an asset from this package.
        /// </summary>
        virtual bool RemoveAsset(
            const Asset::Handle& AssetGuid) = 0;

    protected:
        /// <summary>
        /// Load an asset from this package.
        /// </summary>
        [[nodiscard]] virtual Ptr<IAsset> LoadAsset(
            const Asset::Handle& AssetGuid,
            bool                 LoadTemp) = 0;

        /// <summary>
        /// Unload an asset from this package.
        /// </summary>
        virtual bool UnloadAsset(
            const Asset::Handle& AssetGuid,
            bool                 Force) = 0;

    protected:
        AssetCacheMap             m_Cache;
        mutable std::shared_mutex m_CacheMutex;
    };
} // namespace Neon::Asset