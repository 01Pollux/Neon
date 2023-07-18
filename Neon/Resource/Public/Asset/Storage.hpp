#pragma once

#include <Asset/Handle.hpp>
#include <Asio/ThreadPool.hpp>
#include <future>

namespace Neon::AAsset
{
    class IPackage;
    class IAsset;
    class IAssetHandler;

    //

    class Storage
    {
        using AssetHandlerMap = std::unordered_map<StringU8, UPtr<IAssetHandler>>;

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
        /// Load an asset from a handle.
        /// </summary>
        std::future<Ref<IAsset>> Load(
            IPackage*     Package,
            const Handle& ResHandle);

        /// <summary>
        /// Flush the package.
        /// </summary>
        std::future<void> Flush(
            IPackage* Package);

        /// <summary>
        /// Unload an asset from a handle.
        /// </summary>
        void Unload(
            const Handle& ResHandle);

    private:
        AssetHandlerMap    m_AssetHandlers;
        Asio::ThreadPool<> m_ThreadPool = Asio::ThreadPool(2);
    };
} // namespace Neon::AAsset