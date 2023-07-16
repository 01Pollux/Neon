#pragma once

#include <Core/Neon.hpp>
#include <Asset/Graph.hpp>
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
        virtual Ptr<IAsset> Load(
            IO::InArchive2&       Archive,
            AssetDependencyGraph& Graph) = 0;
    };
} // namespace Neon::AAsset