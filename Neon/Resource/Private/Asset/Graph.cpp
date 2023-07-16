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
        AssetStorage->Load(this->AssetGuid).get();
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
                co_yield BuildTask(PhaseGuid);
                for (auto& Resolver : NodeInfo.Resolvers)
                {
                    std::invoke(std::move(Resolver));
                }
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
                        co_yield BuildTask(Dependent->InMap->first);
                        for (auto& Resolver : Dependent->Resolvers)
                        {
                            std::invoke(std::move(Resolver));
                        }
                    }
                }
            }
        }
    }

    bool AssetDependencyGraph::Requires(
        const Handle& Parent,
        const Handle& Child,
        Ptr<IAsset>&  Asset)
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

        // ParentNodePtr->Resolvers.emplace_back(
        //     [&Asset, &Child](Storage* AssetStorage)
        //     {
        //         Asset = AssetStorage->Load(Child).get().lock();
        //     });
        return false;
    }
} // namespace Neon::AAsset