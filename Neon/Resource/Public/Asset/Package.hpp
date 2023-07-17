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

        using AssetMap = std::unordered_map<Handle, Ptr<IAsset>>;

    public:
        IPackage(
            StringU8 Path) :
            m_PackagePath(std::move(Path))
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
            /// The name of the asset handler.
            /// </summary>
            StringU8 HandlerName;
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
        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<IAsset, _Ty>
        [[nodiscard]] Ptr<IAsset> CreateAsset(
            const AssetAddDesc& Desc,
            _Args&&... Args)
        {
            auto Asset = std::make_shared<_Ty>(std::forward<_Args>(Args)...);
            AddAsset(Desc, Asset);
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
        virtual void Flush(
            Storage* AssetStorage) = 0;

    protected:
        AssetMap   m_AssetCache;
        StringU8   m_PackagePath;
        std::mutex m_PackageMutex;
    };
} // namespace Neon::AAsset