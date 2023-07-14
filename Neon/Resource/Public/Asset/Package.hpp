#pragma once

#include <Core/Neon.hpp>

namespace Neon::AAsset
{
    class IAsset;
    class Storage;
    struct Handle;

    //

    /// <summary>
    /// A package is a collection of assets.
    /// It can be a file, a folder, a zip file...
    /// </summary>
    class IPackage
    {
    public:
        IPackage() = default;
        NEON_CLASS_NO_COPYMOVE(IPackage);
        virtual ~IPackage() = default;

        /// <summary>
        /// Mount the package into asset's storage.
        /// </summary>
        virtual void Mount(
            Storage* Storage) = 0;

        /// <summary>
        /// Unmount the package from asset's storage.
        /// </summary>
        virtual void Unmount(
            Storage* Storage) = 0;

        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        [[nodiscard]] virtual Ptr<IAsset> Load(
            const Handle& Handle) = 0;
    };
} // namespace Neon::AAsset