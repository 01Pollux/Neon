#pragma once

#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <cppcoro/async_manual_reset_event.hpp>
#include <vector>
#include <functional>

namespace Neon::AAsset
{
    class IPackage;
    class Storage;

    class AssetDependencyGraph
    {
    public:
        struct BuildNode;
        struct BuildTask;

        /// <summary>
        /// Compile the graph and return a generator of build tasks.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<BuildTask> Compile();

        /// <summary>
        /// Add a dependency between two assets.
        /// </summary>
        Asio::CoLazy<Ptr<IAsset>> Requires(
            const Handle& Parent,
            const Handle& Child);

    private:
        using BuildNodeMap = std::map<Handle, BuildNode>;
        struct BuildNode;

        BuildNodeMap m_BuildNodes;
    };

    struct AssetDependencyGraph::BuildTask
    {
    public:
        /// <summary>
        /// Resolve the task by loading the asset.
        /// </summary>
        Asio::CoLazy<> Load(
            IPackage* Packge,
            Storage*  AssetStorage);

        BuildTask(
            BuildNode& TargetNode) :
            TargetNode(TargetNode)
        {
        }

    private:
        BuildNode& TargetNode;
    };

    struct AssetDependencyGraph::BuildNode
    {
        Handle AssetHandle;

        std::vector<BuildNode*> DependentNodes;
        size_t                  DependenciesCount = 0;

        Ptr<IAsset>                       Asset;
        cppcoro::async_manual_reset_event ResolvedEvent;
    };
} // namespace Neon::AAsset