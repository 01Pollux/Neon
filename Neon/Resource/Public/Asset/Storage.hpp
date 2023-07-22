#pragma once

#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <future>

namespace Neon::AAsset
{
    class IAsset;
    class IAssetPackage;
    class IAssetHandler;

    struct AddDesc
    {
        /// <summary>
        /// The asset to add.
        /// </summary>
        Ptr<IAsset> Asset;

        /// <summary>
        /// The path to the asset.
        /// </summary>
        StringU8 Path;

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
        static std::future<void> AddAsset(
            const AddDesc& Desc);

        /// <summary>
        /// Removes an asset from the storage system.
        /// </summary>
        static void RemoveAsset(
            const Handle& AssetGuid);

    public:
        /// <summary>
        /// Registers an asset handler.
        /// Not thread safe.
        /// </summary>
        static void RegisterHandler(
            size_t              Id,
            UPtr<IAssetHandler> Handler);

        /// <summary>
        /// Registers an asset handler.
        /// Not thread safe.
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::derived_from<_Ty, IAssetHandler>
        static void RegisterHandler(
            _Args&&... Args)
        {
            RegisterHandler(typeid(_Ty).hash_code(), std::make_unique<_Ty>(std::forward<_Args>(Args)...));
        }

        /// <summary>
        /// Unregisters an asset handler.
        /// Not thread safe.
        /// </summary>
        static void UnregisterHandler(
            size_t Id);

        /// <summary>
        /// Gets the asset handler for the specified asset.
        /// Not thread safe.
        /// </summary>
        static IAssetHandler* GetHandler(
            const Ptr<IAsset>& Asset,
            size_t*            Id = nullptr);

        /// <summary>
        /// Gets the asset handler for the specified asset.
        /// Not thread safe.
        /// </summary>
        static IAssetHandler* GetHandler(
            size_t Id);

    public:
        /// <summary>
        /// Mounts an asset package.
        /// Not thread safe.
        /// </summary>
        static IAssetPackage* Mount(
            UPtr<IAssetPackage> Package);

        /// <summary>
        /// Unmounts an asset package.
        /// Not thread safe.
        /// </summary>
        static void Unmount(
            IAssetPackage* Package);

    public:
        /// <summary>
        /// Gets the asset package with the specified name.
        /// Not thread safe.
        /// </summary>
        [[nodiscard]] static Asio::CoGenerator<IAssetPackage*> GetPackages(
            bool IncludeMemoryOnly = false);

        using PackageAndAsset = std::pair<IAssetPackage*, const Handle&>;

        /// <summary>
        /// Gets all assets in all packages.
        /// Not thread safe.
        /// </summary>
        [[nodiscard]] static Asio::CoGenerator<PackageAndAsset> GetAllAssets(
            bool IncludeMemoryOnly = false);
    };
} // namespace Neon::AAsset