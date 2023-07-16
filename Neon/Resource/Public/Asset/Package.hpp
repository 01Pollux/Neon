#pragma once

#include <Core/Neon.hpp>
#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>

namespace Neon::AAsset
{
    class IAsset;
    class Storage;
    class Storage;

    //

    /// <summary>
    /// A package is a collection of assets.
    /// It can be a file, a folder, a zip file...
    /// </summary>
    class IPackage
    {
        friend class Storage;
        friend class Manager;

    public:
        IPackage() = default;
        NEON_CLASS_NO_COPYMOVE(IPackage);
        virtual ~IPackage() = default;

    protected:
        /// <summary>
        /// Mount the package into asset's storage.
        /// </summary>
        virtual void Mount(
            Storage* AssetStorage) = 0;

        /// <summary>
        /// Unmount the package from asset's storage.
        /// </summary>
        virtual void Unmount(
            Storage* AssetStorage) = 0;

        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        [[nodiscard]] virtual Asio::CoLazy<Ptr<IAsset>> Load(
            Storage*      AssetStorage,
            const Handle& ResHandle) = 0;
    };
} // namespace Neon::AAsset