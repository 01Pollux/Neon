#pragma once

#include <Asset/Handle.hpp>
#include <future>

namespace Neon::Asset
{
    class IAsset;
    class IAssetPackage;

    class Manager
    {
    public:
        /// <summary>
        /// Load asynchronously an asset from the storage system.
        /// </summary>
        static std::future<Ptr<IAsset>> LoadAsync(
            IAssetPackage* Package,
            const Handle&  AssetGuid,
            bool           LoadTemp = false);

        /// <summary>
        /// Load asynchronously an asset from the storage system.
        /// </summary>
        static std::future<Ptr<IAsset>> LoadAsync(
            const Handle& AssetGuid,
            bool          LoadTemp = false);

        /// <summary>
        /// Load an asset from the storage system.
        /// </summary>
        static Ptr<IAsset> Load(
            IAssetPackage* Package,
            const Handle&  AssetGuid,
            bool           LoadTemp = false);

        /// <summary>
        /// Load an asset from the storage system.
        /// </summary>
        static Ptr<IAsset> Load(
            const Handle& AssetGuid,
            bool          LoadTemp = false);

        /// <summary>
        /// Load or reload asynchronously an asset from the storage system.
        /// </summary>
        static std::future<Ptr<IAsset>> ReloadAsync(
            const Handle& AssetGuid);

        /// <summary>
        /// Load or reload an asset from the storage system.
        /// </summary>
        static Ptr<IAsset> Reload(
            const Handle& AssetGuid);

        /// <summary>
        /// Unload an asset from the storage system.
        /// </summary>
        static void Unload(
            const Handle& AssetGuid);

        /// <summary>
        /// Unload an asset from the storage system if it is not referenced anymore.
        /// </summary>
        static void RequestUnload(
            const Handle& AssetGuid);
    };
} // namespace Neon::Asset