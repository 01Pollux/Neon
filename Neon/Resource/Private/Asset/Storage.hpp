#pragma once

#include <Asset/Storage.hpp>
#include <Private/Asset/Manager.hpp>
#include <Asio/ThreadPool.hpp>
#include <mutex>

namespace Neon::Asset
{
    class StorageImpl
    {
        using AssetPackageList = std::vector<UPtr<IAssetPackage>>;

    public:
        /// <summary>
        /// Gets the singleton instance of the storage system.
        /// </summary>
        [[nodiscard]] static StorageImpl* Get();

        /// <summary>
        /// Gets the asset manager.
        /// </summary>
        [[nodiscard]] ManagerImpl* GetManager();

        StorageImpl();
        NEON_CLASS_NO_COPYMOVE(StorageImpl);
        ~StorageImpl();

        /// <summary>
        /// Gets the thread pool used by the storage system.
        /// </summary>
        Asio::ThreadPool<>& GetThreadPool();

    public:
        /// <summary>
        /// Adds an asset to the storage system.
        /// </summary>
        std::future<void> SaveAsset(
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
        void RegisterHandler(
            size_t              Id,
            UPtr<IAssetHandler> Handler);

        /// <summary>
        /// Unregisters an asset handler.
        /// Not thread safe.
        /// </summary>
        void UnregisterHandler(
            size_t Id);

        /// <summary>
        /// Gets the asset handler for the specified asset.
        /// Not thread safe.
        /// </summary>
        IAssetHandler* GetHandler(
            const Ptr<IAsset>& Asset,
            size_t*            Id);

        /// <summary>
        /// Gets the asset handler for the specified asset.
        /// Not thread safe.
        /// </summary>
        IAssetHandler* GetHandler(
            size_t Id);

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
        AssetPackageList                      m_Packages;
        std::map<size_t, UPtr<IAssetHandler>> m_Handlers;

        ManagerImpl        m_Manager;
        Asio::ThreadPool<> m_ThreadPool;
    };
} // namespace Neon::Asset