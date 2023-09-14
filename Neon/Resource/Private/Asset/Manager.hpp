#pragma once

#include <Asset/Manager.hpp>

namespace Neon::Asset
{
    class ManagerImpl
    {
    public:
        /// <summary>
        /// Get the asset manager.
        /// </summary>
        [[nodiscard]] static ManagerImpl* Get();

    public:
        /// <summary>
        /// Load asynchronously an asset from the storage system.
        /// </summary>
        std::future<Ptr<IAsset>> Load(
            const Handle& AssetGuid);

        /// <summary>
        /// Load or reload asynchronously an asset from the storage system.
        /// </summary>
        std::future<Ptr<IAsset>> Reload(
            const Handle& AssetGuid);

        /// <summary>
        /// Unload an asset from the storage system.
        /// </summary>
        bool Unload(
            const Handle& AssetGuid);

        /// <summary>
        /// Unload an asset from the storage system if it is not referenced anymore.
        /// </summary>
        bool RequestUnload(
            const Handle& AssetGuid);
    };
} // namespace Neon::Asset