#pragma once

#include <Asset/Storage.hpp>
#include <vector>

namespace Neon::AAsset
{
    class StorageImpl
    {
        using AssetPackageList = std::vector<UPtr<IAssetPackage>>;

    public:
        [[nodiscard]] static StorageImpl* Get();

        StorageImpl();

        /// <summary>
        /// Adds an asset to the storage system.
        /// </summary>
        void AddAsset(
            const AddDesc& Desc);

        /// <summary>
        /// Removes an asset from the storage system.
        /// </summary>
        void RemoveAsset(
            const Handle& AssetGuid);

    public:
        /// <summary>
        /// Mounts an asset package.
        /// This function is not thread-safe.
        /// </summary>
        IAssetPackage* Mount(
            UPtr<IAssetPackage> Package);

        /// <summary>
        /// Unmounts an asset package.
        /// This function is not thread-safe.
        /// </summary>
        void Unmount(
            IAssetPackage* Package);

    public:
        Asio::CoGenerator<IAssetPackage*> GetPackages(
            bool IncludeMemoryOnly);

        Asio::CoGenerator<Storage::PackageAndAsset> GetAllAssets(
            bool IncludeMemoryOnly);

    private:
        AssetPackageList m_Packages;
    };
} // namespace Neon::AAsset