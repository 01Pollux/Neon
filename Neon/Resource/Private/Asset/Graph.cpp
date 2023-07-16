#include <ResourcePCH.hpp>
#include <Asset/Graph.hpp>
#include <Asset/Storage.hpp>

#include <queue>

#include <Log/Logger.hpp>

namespace Neon::AAsset
{
    Asio::CoLazy<> AssetDependencyGraph::BuildTask::Resolve(
        Storage* AssetStorage)
    {
        TargetNode.Asset = AssetStorage->Load(TargetNode.InMap->first).get().lock();
        co_return;
    }

    auto AssetDependencyGraph::Compile() -> Asio::CoGenerator<BuildTask>
    {
        std::queue<BuildNode*> CurrentLevel;

        for (auto& [PhaseGuid, NodeInfo] : m_BuildNodes)
        {
            auto Iter = &m_BuildNodes[PhaseGuid];
            if (!NodeInfo.DependenciesCount)
            {
                if (!NodeInfo.DependentNodes.empty())
                {
                    CurrentLevel.push(&NodeInfo);
                }
                co_yield BuildTask(NodeInfo);

                printf("leaf-signaling event for %p\n", &NodeInfo);
                NodeInfo.ResolvedEvent.set();
            }
        }

        while (!CurrentLevel.empty())
        {
            size_t Size = CurrentLevel.size();
            for (size_t i = 0; i < Size; i++)
            {
                auto CurrentPhase = CurrentLevel.front();
                CurrentLevel.pop();

                for (auto& Dependent : CurrentPhase->DependentNodes)
                {
                    Dependent->DependenciesCount--;
                    if (!Dependent->DependenciesCount)
                    {
                        if (!Dependent->DependentNodes.empty())
                        {
                            CurrentLevel.push(Dependent);
                        }
                        co_yield BuildTask(*Dependent);

                        printf("node-signaling event for %p\n", Dependent);
                        Dependent->ResolvedEvent.set();
                    }
                }
            }
        }
    }

    Asio::CoLazy<Ptr<IAsset>> AssetDependencyGraph::Requires(
        const Handle& Parent,
        const Handle& Child)
    {
        auto ParentNode = m_BuildNodes.emplace(std::piecewise_construct, std::forward_as_tuple(Parent), std::forward_as_tuple()).first;
        auto ChildNode  = m_BuildNodes.emplace(std::piecewise_construct, std::forward_as_tuple(Child), std::forward_as_tuple()).first;

        auto ParentNodePtr = &ParentNode->second;
        auto ChildNodePtr  = &ChildNode->second;

        // Detect circular dependencies
        auto PendingPhases = ParentNodePtr->DependentNodes;
        while (!PendingPhases.empty())
        {
            auto CheckPhase = PendingPhases.back();
            PendingPhases.pop_back();
            NEON_VALIDATE(CheckPhase != ParentNodePtr, "Circular dependency detected between assets '{}' and '{}'", Parent.ToString(), Child.ToString());

            PendingPhases.insert_range(PendingPhases.end(), CheckPhase->DependentNodes);
        }

        ChildNodePtr->DependentNodes.push_back(ParentNodePtr);
        ParentNodePtr->DependenciesCount++;
        ParentNodePtr->InMap = ParentNode;

        co_await ChildNodePtr->ResolvedEvent;
        co_return ChildNodePtr->Asset;
    }
} // namespace Neon::AAsset