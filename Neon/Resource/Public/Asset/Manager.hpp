#pragma once

#include <Core/Neon.hpp>
#include <Core/String.hpp>

#include <Asset/Handle.hpp>

namespace Neon::AAsset
{
    using PackageHandle = size_t;

    class IPackage;
    class IAsset;

    //

    class Manager
    {
    public:
        enum class MountType : uint8_t
        {
            Directory,
            Zip,
            Database
        };

        /// <summary>
        /// Mount a package from a path.
        /// </summary>
        [[nodiscrad]] PackageHandle Mount(
            const StringU8& Path,
            MountType       Type);

        /// <summary>
        /// Unmount a package from a handle.
        /// </summary>
        void Unmount(
            PackageHandle Package);

        /// <summary>
        /// Get a package from a handle.
        /// </summary>
        Ref<IPackage> GetPackage(
            PackageHandle Package);

        //

        /// <summary>
        /// Preload an asset from a handle.
        /// </summary>
        void LoadAsync(
            const Handle& Handle);

        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        [[nodiscrad]] Ref<IAsset> Load(
            const Handle& Handle);

        /// <summary>
        /// Unload an asset from a handle.
        /// </summary>
        void Unload(
            const Handle& Handle);

    private:
        std::map<PackageHandle, Ptr<IPackage>> m_Packages;
    };
} // namespace Neon::AAsset