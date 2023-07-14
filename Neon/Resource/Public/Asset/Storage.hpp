#pragma once

#include <Asset/Handle.hpp>

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

    public:
        /// <summary>
        /// Preload an asset from a handle.
        /// </summary>
        void LoadAsync(
            const Handle& Handle);

        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        [[nodiscard]] Ref<IAsset> Load(
            const Handle& Handle);

        /// <summary>
        /// Unload an asset from a handle.
        /// </summary>
        void Unload(
            const Handle& Handle);

    public:
        /// <summary>
        /// Reference an asset from a handle.
        /// </summary>
        void Reference(
            IPackage*     Package,
            const Handle& Handle);

        /// <summary>
        /// Reference an asset from a handle.
        /// </summary>
        void Unreference(
            IPackage*     Package,
            const Handle& Handle);

    private:
        AssetMap          m_AssetCache;
        AssetMapInPackage m_AssetsInPackage;
        AssetHandlerMap   m_AssetHandlers;
    };
} // namespace Neon::AAsset