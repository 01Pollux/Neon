#pragma once

#include <Asset/Handle.hpp>
#include <Asio/Coroutines.hpp>
#include <cppcoro/async_manual_reset_event.hpp>
#include <vector>
#include <unordered_set>
#include <queue>
#include <stack>

namespace Neon::AAsset
{
    class IPackage;
    class Storage;

    class AssetDependencyGraph
    {
    public:
        struct BuildTask;

        /// <summary>
        /// Resolve the graph and return the asset.
        /// </summary>
        [[nodiscard]] Ptr<IAsset> Resolve();

        /// <summary>
        /// Compile the graph and return a generator of build tasks.
        /// </summary>
        [[nodiscard]] Asio::CoGenerator<BuildTask> Compile();

        /// <summary>
        /// Add a dependency between two assets.
        /// the child mustn't be null.
        /// </summary>
        Asio::CoLazy<Ptr<IAsset>> Requires(
            const Handle& Parent,
            const Handle& Child);

        /// <summary>
        /// Add a dependency between assets.
        /// the child mustn't be null.
        /// </summary>
        Asio::CoLazy<Ptr<IAsset>> Requires(
            const Handle&           Parent,
            std::span<const Handle> Children);

    private:
        struct TaskNode
        {
            Ptr<IAsset> Asset = nullptr;

            cppcoro::async_manual_reset_event Event;
        };

        using RequiredTaskStack  = std::stack<Handle>;
        using LoadedTaskEventMap = std::unordered_map<Handle, TaskNode>;

        RequiredTaskStack  m_RequiredTasks;
        LoadedTaskEventMap m_LoadedTasks;
    };

    struct AssetDependencyGraph::BuildTask
    {
    public:
        /// <summary>
        /// Resolve the task by loading the asset.
        /// </summary>
        Asio::CoLazy<> Load(
            const std::function<Ptr<IAsset>(const Handle& AssetGuid)>& LoadAssetFunc);

        BuildTask(
            const Handle& AssetGuid,
            TaskNode&     Node) :
            AssetGuid(AssetGuid),
            Node(Node)
        {
        }

    private:
        Handle    AssetGuid;
        TaskNode& Node;
    };
} // namespace Neon::AAsset