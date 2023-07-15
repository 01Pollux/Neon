#pragma once

#include <Asset/Handle.hpp>

#include <cppcoro/task.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/async_mutex.hpp>

namespace Neon::AAsset
{
    class IPackage;
    class IAsset;
    class IAssetHandler;

    //

    class Storage
    {
        using AssetMap          = std::unordered_map<Handle, Ptr<IAsset>>;
        using AssetMapInPackage = std::unordered_map<Handle, IPackage*>;
        using AssetHandlerMap   = std::unordered_map<StringU8, UPtr<IAssetHandler>>;

    public:
        /// <summary>
        /// Register an asset handler.
        /// </summary>
        void RegisterHandler(
            const StringU8&     Name,
            UPtr<IAssetHandler> Handler);

        /// <summary>
        /// Unregister an asset handler.
        /// </summary>
        void UnregisterHandler(
            const StringU8& Name);

        /// <summary>
        /// Get an asset handler.
        /// </summary>
        IAssetHandler* GetHandler(
            const StringU8& Name);

    public:
        /// <summary>
        /// Preload an asset from a handle.
        /// </summary>
        void LoadAsync(
            const Handle& ResHandle);

        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        [[nodiscard]] cppcoro::task<Ref<IAsset>> Load(
            const Handle& ResHandle);

        /// <summary>
        /// Unload an asset from a handle.
        /// </summary>
        void Unload(
            const Handle& ResHandle);

    public:
        /// <summary>
        /// Reference an asset from a handle.
        /// </summary>
        void Reference(
            IPackage*     Package,
            const Handle& ResHandle);

        /// <summary>
        /// Reference an asset from a handle.
        /// </summary>
        void Unreference(
            IPackage*     Package,
            const Handle& ResHandle);

    private:
        AssetMap          m_AssetCache;
        AssetMapInPackage m_AssetsInPackage;
        AssetHandlerMap   m_AssetHandlers;

        cppcoro::static_thread_pool m_ThreadPool{ 2 };
        cppcoro::async_mutex        m_AssetCacheMutex;
        cppcoro::async_mutex        m_AssetsInPackageMutex;
    };
} // namespace Neon::AAsset