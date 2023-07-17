#pragma once

#include <Asset/Graph.hpp>
#include <Asio/Coroutines.hpp>
#include <IO/Archive2.hpp>

namespace Neon::AAsset
{
    class IAsset;
    class IAssetHandler
    {
    public:
        virtual ~IAssetHandler() = default;

        /// <summary>
        /// Load an asset from an archive while building the dependency graph.
        /// </summary>
        virtual Asio::CoLazy<Ptr<IAsset>> Load(
            IO::InArchive2&       Archive,
            const AAsset::Handle& AssetGuid,
            AssetDependencyGraph& Graph) = 0;

        /// <summary>
        /// Save an asset
        /// </summary>
        virtual void Save(
            IO::OutArchive2&   Archive,
            const Ptr<IAsset>& Asset) = 0;
    };
} // namespace Neon::AAsset