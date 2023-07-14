#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>

#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    class IPackage;
    class IAsset;

    //

    class IManager
    {
    public:
        /// <summary>
        /// Mount a package from a file path.
        /// </summary>
        virtual Ref<IPackage> Mount(
            const StringU8& Path) = 0;

        /// <summary>
        /// Open a package from a file path.
        /// </summary>
        virtual void Unmount(
            Ref<IPackage> Package) = 0;

        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        virtual Ref<IAsset> Load(
            const Handle&   Handle,
            const StringU8& Path,
            const StringU8& Name,
            const StringU8& Extension) = 0;
    };
} // namespace Neon::AAsset