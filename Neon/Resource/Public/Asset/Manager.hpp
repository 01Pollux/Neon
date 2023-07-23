#pragma once

#include <Asset/Handle.hpp>
#include <future>

namespace Neon::Asset
{
    class IAsset;

    class Manager
    {
    public:
        /// <summary>
        /// Load asynchronously an asset from the storage system.
        /// </summary>
        static std::future<Ptr<IAsset>> Load(
            const Handle& AssetGuid);

        /// <summary>
        /// Load or reload asynchronously an asset from the storage system.
        /// </summary>
        static std::future<Ptr<IAsset>> Reload(
            const Handle& AssetGuid);

        /// <summary>
        /// Unload an asset from the storage system.
        /// </summary>
        static void Unload(
            const Handle& AssetGuid);
    };
} // namespace Neon::Asset