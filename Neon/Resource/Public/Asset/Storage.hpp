#pragma once

#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>

namespace Neon::AAsset
{
    class IAsset;
    class IAssetPackage;

    struct AddDesc
    {
        /// <summary>
        /// The asset to add.
        /// </summary>
        Ptr<IAsset> Asset;

        /// <summary>
        /// The package to add the asset to.
        /// If null, the asset will be added to the first package.
        /// </summary>
        IAssetPackage* PreferredPackage = nullptr;

        /// <summary>
        /// If true, the asset will be stored in memory only.
        /// </summary>
        bool MemoryOnly : 1 = false;
    };

    class Storage
    {
    public:
        /// <summary>
        /// Initializes the asset storage system.
        /// </summary>
        static void Initialize();

        /// <summary>
        /// Shuts down the asset storage system.
        /// </summary>
        static void Shutdown();

        /// <summary>
        /// Adds an asset to the storage system.
        /// </summary>
        static void AddAsset(
            const AddDesc& Desc);

        /// <summary>
        /// Removes an asset from the storage system.
        /// </summary>
        static void RemoveAsset(
            const Handle& AssetGuid);

    public:
        /// <summary>
        /// Mounts an asset package.
        /// This function is not thread-safe.
        /// </summary>
        static IAssetPackage* Mount(
            UPtr<IAssetPackage> Package);

        /// <summary>
        /// Unmounts an asset package.
        /// This function is not thread-safe.
        /// </summary>
        static void Unmount(
            IAssetPackage* Package);

    public:
        /// <summary>
        /// Gets the asset package with the specified name.
        /// </summary>
        [[nodiscard]] static Asio::CoGenerator<IAssetPackage*> GetPackages(
            bool IncludeMemoryOnly = false);

        using PackageAndAsset = std::pair<IAssetPackage*, const Handle&>;

        /// <summary>
        /// Gets all assets in all packages.
        /// </summary>
        [[nodiscard]] static Asio::CoGenerator<PackageAndAsset> GetAllAssets(
            bool IncludeMemoryOnly = false);
    };
} // namespace Neon::AAsset