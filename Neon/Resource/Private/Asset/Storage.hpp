#pragma once

#include <Asset/Storage.hpp>
#include <Asio/ThreadPool.hpp>
#include <mutex>

namespace Neon::AAsset
{
    class StorageImpl
    {
        using AssetPackageList = std::vector<UPtr<IAssetPackage>>;

    public:
        [[nodiscard]] static StorageImpl* Get();

        StorageImpl();

        /// <summary>
        /// Gets the thread pool used by the storage system.
        /// </summary>
        Asio::ThreadPool<>& GetThreadPool();

    public:
        /// <summary>
        /// Adds an asset to the storage system.
        /// </summary>
        std::future<void> AddAsset(
            const AddDesc& Desc);

        /// <summary>
        /// Removes an asset from the storage system.
        /// </summary>
        void RemoveAsset(
            const Handle& AssetGuid);

    public:
        /// <summary>
        /// Registers an asset handler.
        /// Not thread safe.
        /// </summary>
        IAssetHandler* RegisterHandler(
            UPtr<IAssetHandler> Handler);

        /// <summary>
        /// Unregisters an asset handler.
        /// Not thread safe.
        /// </summary>
        void UnregisterHandler(
            IAssetHandler* Handler);

        /// <summary>
        /// Gets the asset handler for the specified asset.
        /// Not thread safe.
        /// </summary>
        IAssetHandler* GetHandler(
            const Ptr<IAsset>& Asset);

    public:
        /// <summary>
        /// Mounts an asset package.
        /// Not thread safe.
        /// </summary>
        IAssetPackage* Mount(
            UPtr<IAssetPackage> Package);

        /// <summary>
        /// Unmounts an asset package.
        /// Not thread safe.
        /// </summary>
        void Unmount(
            IAssetPackage* Package);

    public:
        /// <summary>
        /// Gets the asset package with the specified name.
        /// Not thread safe.
        /// </summary>
        Asio::CoGenerator<IAssetPackage*> GetPackages(
            bool IncludeMemoryOnly);

        /// <summary>
        /// Gets all assets in all packages.
        /// Not thread safe.
        /// </summary>
        Asio::CoGenerator<Storage::PackageAndAsset> GetAllAssets(
            bool IncludeMemoryOnly);

    private:
        AssetPackageList                 m_Packages;
        Asio::ThreadPool<>               m_ThreadPool;
        std::vector<UPtr<IAssetHandler>> m_Handlers;
    };
} // namespace Neon::AAsset