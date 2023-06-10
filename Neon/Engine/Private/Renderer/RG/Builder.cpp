#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Builder.hpp>

namespace Neon::RG
{
    RenderGraph::Builder::Builder(
        RenderGraph& Context) :
        m_Context(Context)
    {
        // AppendPass<InitializeBackbufferPass>();
    }

    void RenderGraph::Builder::Build()
    {
        // AppendPass<FinalizeBackbufferPass>();

        //

        auto LoadedShaders = std::make_shared<LockableData<ShaderMapType>>();
        auto Builders      = std::make_shared<BuildersListType>();

        Builders->reserve(m_Passes.size());
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            Builders->emplace_back(m_Context.GetStorage(), LoadedShaders->GetUnsafe());
        }

        // Launch pipeline state jobs
        m_Context.m_PipelineCreators = LaunchPipelineJobs(Builders, LoadedShaders);

        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Builder = (*Builders)[i];
            m_Passes[i]->SetupResources(Builder.Resources);
        }
        m_Context.Build(BuildPasses(Builders));
    }

    //

    auto RenderGraph::Builder::BuildPasses(
        const std::shared_ptr<BuildersListType>& Builders) -> std::vector<DepdencyLevel>
    {
        BuildAdjacencyLists(*Builders);
        TopologicalSort();
        auto ResourcesToDestroy = CalculateResourcesLifetime(*Builders);
        return BuildDependencyLevels(*Builders, ResourcesToDestroy);
    }

    //

    void RenderGraph::Builder::BuildAdjacencyLists(
        const BuildersListType& Builders)
    {
        m_AdjacencyList.resize(m_Passes.size());
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Adjacencies = m_AdjacencyList[i];
            auto& PassBuilder = Builders[i].Resources;

            for (size_t j = i + 1; j < m_Passes.size(); j++)
            {
                auto& OtherPassBuilder = Builders[j].Resources;
                bool  Depends          = false;

                for (auto& OtherPassRead : OtherPassBuilder.m_ResourcesRead)
                {
                    if (PassBuilder.m_ResourcesWritten.contains(OtherPassRead))
                    {
                        Adjacencies.push_back(j);
                        Depends = true;
                        break;
                    }
                }
            }
        }
    }

    //

    void RenderGraph::Builder::TopologicalSort()
    {
        std::stack<size_t> Stack{};
        std::vector<bool>  Visited(m_Passes.size(), false);
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            if (!Visited[i])
            {
                DepthFirstSearch(i, Visited, Stack);
            }
        }

        m_TopologicallySortedList.reserve(Stack.size());
        while (!Stack.empty())
        {
            m_TopologicallySortedList.push_back(Stack.top());
            Stack.pop();
        }
    }

    //

    std::vector<std::set<ResourceId>> RenderGraph::Builder::CalculateResourcesLifetime(
        BuildersListType& Builders) const
    {
        std::vector<std::set<ResourceId>> ResourceToDestroy(m_Passes.size());
        std::map<ResourceId, size_t>      LastUsedResources;

        for (size_t i = 0; i < m_TopologicallySortedList.size(); i++)
        {
            size_t PassIdx   = m_TopologicallySortedList[i];
            auto&  Resources = Builders[PassIdx].Resources;

            for (auto& ResId : Resources.m_ResourcesWritten)
            {
                LastUsedResources[ResId] = PassIdx;
            }
            for (auto& ResId : Resources.m_ResourcesRead)
            {
                LastUsedResources[ResId] = PassIdx;
            }
        }

        for (auto& [Id, PassIdx] : LastUsedResources)
        {
            ResourceToDestroy[PassIdx].insert(Id);
        }

        return ResourceToDestroy;
    }

    //

    void RenderGraph::Builder::DepthFirstSearch(
        size_t              Index,
        std::vector<bool>&  Visited,
        std::stack<size_t>& Stack)
    {
        Visited[Index] = true;
        for (size_t AdjIndex : m_AdjacencyList[Index])
        {
            if (!Visited[AdjIndex])
            {
                DepthFirstSearch(AdjIndex, Visited, Stack);
            }
        }
        Stack.push(Index);
    }

    //

    auto RenderGraph::Builder::BuildDependencyLevels(
        BuildersListType&                  Builders,
        std::vector<std::set<ResourceId>>& ResourceToDestroy) -> std::vector<DepdencyLevel>
    {
        std::vector<size_t> Distances(m_TopologicallySortedList.size());
        for (size_t d = 0; d < Distances.size(); d++)
        {
            size_t i = m_TopologicallySortedList[d];
            for (size_t AdjIndex : m_AdjacencyList[i])
            {
                if (Distances[AdjIndex] < (Distances[i] + 1))
                {
                    Distances[AdjIndex] = Distances[i] + 1;
                }
            }
        }

        size_t Size = std::ranges::max(Distances) + 1;

        std::vector<DepdencyLevel> Dependencies;
        Dependencies.reserve(Size);

        for (size_t i = 0; i < Size; i++)
        {
            Dependencies.emplace_back(m_Context);
        }

        for (size_t i = 0; i < m_Passes.size(); ++i)
        {
            size_t Level = Distances[i];

            Dependencies[Level].AddPass(
                std::move(m_Passes[i]),
                std::move(Builders[i].Resources.m_RenderTargets),
                std::move(Builders[i].Resources.m_DepthStencil),
                std::move(Builders[i].Resources.m_ResourcesCreated),
                std::move(ResourceToDestroy[i]),
                std::move(Builders[i].Resources.m_ResourceStates));
        }

        return Dependencies;
    }

    //

    IRenderPass& RenderGraph::Builder::AppendPass(
        std::unique_ptr<IRenderPass> Pass)
    {
        return *m_Passes.emplace_back(std::move(Pass));
    }

    RenderGraph::Builder::BuilderInfo::BuilderInfo(
        GraphStorage& Storage) :
        Resources(Storage)
    {
    }
} // namespace Neon::RG
