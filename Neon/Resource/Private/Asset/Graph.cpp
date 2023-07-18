#include <ResourcePCH.hpp>
#include <Asset/Graph.hpp>
#include <Asset/Storage.hpp>
#include <Asset/Package.hpp>

#include <queue>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    Asio::CoLazy<> AssetDependencyGraph::BuildTask::Load(
        const std::function<Ptr<IAsset>(const Handle& AssetGuid)>& AssetLoadFunc)
    {
        Node.Asset = AssetLoadFunc(this->AssetGuid);
        co_return;
    }

    Ptr<IAsset> AssetDependencyGraph::Resolve()
    {
    }

    auto AssetDependencyGraph::Compile() -> Asio::CoGenerator<BuildTask>
    {
        while (!m_RequiredTasks.empty())
        {
            auto ToLoad = m_RequiredTasks.top();
            m_RequiredTasks.pop();

            auto& Node = m_LoadedTasks[ToLoad];

            // If the asset is already loaded, skip it.
            if (!Node.Asset)
            {
                NEON_TRACE_TAG("Asset", "AssetDependencyGraph::Compile: Loading: {}", ToLoad.ToString());

                co_yield BuildTask(ToLoad, Node);
                Node.Event.set();
            }
        }
    }

    Asio::CoLazy<Ptr<IAsset>> AssetDependencyGraph::Requires(
        const Handle& Parent,
        const Handle& Child)
    {
        NEON_TRACE_TAG("Asset", "AssetDependencyGraph::Requires: Parent: {}, Child: {}", Parent.ToString(), Child.ToString());

        m_RequiredTasks.push(Parent);
        m_RequiredTasks.push(Child);

        auto& Node = m_LoadedTasks[Parent];
        co_await Node.Event;
        co_return Node.Asset;
    }

    Asio::CoLazy<Ptr<IAsset>> AssetDependencyGraph::Requires(
        const Handle&           Parent,
        std::span<const Handle> Children)
    {
        m_RequiredTasks.push(Parent);
        for (const auto& Child : Children)
        {
            NEON_TRACE_TAG("Asset", "AssetDependencyGraph::Requires: Parent: {}, Child: {}", Parent.ToString(), Child.ToString());
            m_RequiredTasks.push(Child);
        }

        auto& Node = m_LoadedTasks[Parent];
        co_await Node.Event;
        co_return Node.Asset;
    }
} // namespace Neon::AAsset