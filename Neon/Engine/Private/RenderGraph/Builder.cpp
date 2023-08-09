#include <EnginePCH.hpp>
#include <RenderGraph/RG.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    class InitializeOutputImage : public IRenderPass
    {
    public:
        InitializeOutputImage() :
            IRenderPass("InitializeOutputImage", PassQueueType::Unknown)
        {
        }

        void ResolveResources(
            ResourceResolver& Resolver) override
        {
            auto Desc = RHI::ResourceDesc::Tex2D(
                ResourceResolver::GetSwapchainFormat(),
                0, 0, 1, 1);
            Desc.SetClearValue(Colors::Green);

            Resolver.CreateTexture(
                RG::ResourceId(STR("OutputImage")),
                Desc,
                MResourceFlags::FromEnum(EResourceFlags::WindowSizeDependent));
        }
    };

    //

    GraphBuilder::GraphBuilder(
        RenderGraph& Context) :
        m_Context(Context)
    {
        AddPass<InitializeOutputImage>();
    }

    void GraphBuilder::Build()
    {
        BuildersListType Builders;

        Builders.reserve(m_Passes.size());
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            Builders.emplace_back(m_Context.GetStorage());
        }

        auto& Storage = m_Context.GetStorage();
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Builder = Builders[i];
            m_Passes[i]->ResolveResources(Builder.Resources);
        }
        m_Context.Build(BuildPasses(Builders));
    }

    auto GraphBuilder::BuildPasses(
        BuildersListType& Builders) -> std::vector<GraphDepdencyLevel>
    {
        BuildAdjacencyLists(Builders);
        TopologicalSort();
        auto ResourcesToDestroy = CalculateResourcesLifetime(Builders);
        return BuildDependencyLevels(Builders, ResourcesToDestroy);
    }

    //

    void GraphBuilder::BuildAdjacencyLists(
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

    void GraphBuilder::TopologicalSort()
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

    auto GraphBuilder::CalculateResourcesLifetime(
        BuildersListType& Builders) const -> std::vector<std::set<ResourceId>>
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

    void GraphBuilder::DepthFirstSearch(
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

    auto GraphBuilder::BuildDependencyLevels(
        BuildersListType&                  Builders,
        std::vector<std::set<ResourceId>>& ResourceToDestroy) -> std::vector<GraphDepdencyLevel>
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

        std::vector<GraphDepdencyLevel> Dependencies;
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

    IRenderPass& GraphBuilder::AddPass(
        std::unique_ptr<IRenderPass> Pass)
    {
        return *m_Passes.emplace_back(std::move(Pass));
    }

    GraphBuilder::BuilderInfo::BuilderInfo(
        GraphStorage& Storage) :
        Resources(Storage)
    {
    }
} // namespace Neon::RG
