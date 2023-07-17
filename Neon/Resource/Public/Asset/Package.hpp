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
        IPackage(
            const std::filesystem::path& Path) :
            m_PackagePath(Path)
        {
        }

        NEON_CLASS_NO_COPYMOVE(IPackage);
        virtual ~IPackage() = default;

        struct AssetAddDesc
        {
            /// <summary>
            /// The path of the asset in the package.
            /// </summary>
            StringU8 Path;

            /// <summary>
            /// If empty, the handler will be chosen automatically.
            /// </summary>
            StringU8 HandlerName = "";
        };

        /// <summary>
        /// Create an aaset from this package.
        /// </summary>
        [[nodiscard]] virtual void AddAsset(
            const AssetAddDesc& Desc,
            const Ptr<IAsset>&  Asset) = 0;

        /// <summary>
        /// Remove an asset from this package.
        /// </summary>
        [[nodiscard]] virtual void RemoveAsset(
            const Handle& AssetHandle) = 0;

        /// <summary>
        /// Create an aaset from this package.
        /// </summary>
        template<typename... _Args>
        [[nodiscard]] Ptr<IAsset> CreateAsset(
            const AssetAddDesc& Desc,
            _Args&&... Args)
        {
            auto Asset = std::make_shared<IAsset>(std::forward<_Args>(Args)...);
            CreateAsset(Desc, Asset);
            return Asset;
        }

    protected:
        /// <summary>
        /// Load an asset from a handle.
        /// </summary>
        [[nodiscard]] virtual Asio::CoLazy<Ptr<IAsset>> Load(
            Storage*      AssetStorage,
            const Handle& ResHandle) = 0;

        /// <summary>
        /// Flush the package.
        /// </summary>
        virtual void Flush() = 0;

    protected:
        std::filesystem::path m_PackagePath;
        std::mutex            m_PackageMutex;
    };
} // namespace Neon::AAsset