#pragma once

#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <vector>
#include <functional>

namespace Neon::AAsset
{
    class Storage;

    class AssetDependencyGraph
    {
    public:
        struct BuildTask
        {
            /// <summary>
            /// Resolve the task by loading the asset.
            /// </summary>
            Asio::CoLazy<> Resolve(
                Storage* AssetStorage);

            BuildTask(
                const Handle& AssetGuid) :
                AssetGuid(AssetGuid)
            {
            }

            const Handle& AssetGuid;
        };

        /// <summary>
        /// Compile the graph and return a generator of build tasks.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<BuildTask> Compile();

        /// <summary>
        /// Add a dependency between two assets.
        /// </summary>
        bool Requires(
            const Handle& Parent,
            const Handle& Child,
            Ptr<IAsset>&  Asset);

    private:
        using ResolverFunctions = std::vector<std::function<void()>>;

        struct BuildNode;
        using BuildNodeMap = std::map<Handle, BuildNode>;
        struct BuildNode
        {
            BuildNodeMap::iterator  InMap;
            std::vector<BuildNode*> DependentNodes;
            size_t                  DependenciesCount = 0;
            ResolverFunctions       Resolvers;
        };

        BuildNodeMap m_BuildNodes;
    };
} // namespace Neon::AAsset