#pragma once

#include <Core/Neon.hpp>
#include <Core/BitMask.hpp>
#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <shared_mutex>

namespace Neon::AAsset
{
    class IAsset;
    class Storage;
    class Storage;

    //

    enum class EAssetFlags : uint8_t
    {
        /// <summary>
        /// The asset is dirty and needs to be saved.
        /// </summary>
        Dirty,

        /// <summary>
        /// The asset is loaded in memory hence won't be saved.
        /// </summary>
        MemoryOnly,

        /// <summary>
        /// The asset is always saved regardless of its dirty state.
        /// </summary>
        AlwaysSave,

        _Last_Enum
    };
    using MAAssetFlags = Bitmask<EAssetFlags>;

    /// <summary>
    /// A package is a collection of assets.
    /// It can be a file, a folder, a zip file...
    /// </summary>
    class IPackage
    {
        friend class Storage;
        friend class Manager;
        friend class AssetDependencyGraph;

        struct AssetCache
        {
            Ptr<IAsset>  Asset;
            MAAssetFlags Flags;
        };

        using AssetCacheMap = std::unordered_map<Handle, Ptr<IAsset>>;

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
        virtual void AddAsset(
            const AssetAddDesc& Desc,
            const Ptr<IAsset>&  Asset) = 0;

        /// <summary>
        /// Remove an asset from this package.
        /// </summary>
        virtual void RemoveAsset(
            const Handle& AssetHandle) = 0;

        /// <summary>
        /// Create an aaset from this package.
        /// </summary>
        template<typename _Ty, typename... _Args>
            requires std::is_base_of_v<IAsset, _Ty>
        Ptr<_Ty> CreateAsset(
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
        [[nodiscard]] virtual Ptr<IAsset> Load(
            Storage*      AssetStorage,
            const Handle& ResHandle) = 0;

        /// <summary>
        /// Flush the package.
        /// </summary>
        virtual void Flush(
            Storage* AssetStorage) = 0;

        /// <summary>
        /// Get all assets.
        /// </summary>
        [[nodiscard]] virtual std::vector<Handle> GetAssets() const = 0;

    protected:
        AssetCacheMap     m_AssetCache;
        std::shared_mutex m_AssetsMutex;
        StringU8          m_PackagePath;
    };
} // namespace Neon::AAsset