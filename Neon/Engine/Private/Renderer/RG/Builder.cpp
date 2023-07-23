#include <EnginePCH.hpp>
#include <Renderer/RG/RG.hpp>
#include <Renderer/RG/Passes/OuputImage.hpp>

#include <Log/Logger.hpp>

namespace Neon::RG
{
    RenderGraphBuilder::RenderGraphBuilder(
        RenderGraph& Context) :
        m_Context(Context)
    {
        AppendPass<InitializeOutputImage>();
    }

    void RenderGraphBuilder::Build()
    {
        BuildersListType Builders;

        Builders.reserve(m_Passes.size());
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            Builders.emplace_back(m_Context.GetStorage());
        }

        LaunchPipelineJobs(Builders);

        auto& Storage = m_Context.GetStorage();
        for (size_t i = 0; i < m_Passes.size(); i++)
        {
            auto& Builder = Builders[i];
            m_Passes[i]->ResolveResources(Builder.Resources);
        }
        m_Context.Build(BuildPasses(Builders));
    }

    auto RenderGraphBuilder::LaunchRootSignatureJobs(
        BuildersListType& Builders) -> void
    {
        for (size_t i = 0; i < Builders.size(); i++)
        {
            auto& Builder = Builders[i];
            m_Passes[i]->ResolveRootSignature(Builder.RootSignatures);
        }
        for (auto& Builder : Builders)
        {
            for (auto& [Id, Desc] : Builder.RootSignatures.m_RootSignaturesToLoad)
            {
                std::visit(
                    VariantVisitor{
                        [&Id, &Storage = m_Context.GetStorage()](const auto& Builder)
                        {
                            Storage.ImportRootSignature(Id, RHI::IRootSignature::Create(Builder));
                        },
                        [&Id, &Storage = m_Context.GetStorage()](const Ptr<RHI::IRootSignature>& RootSig)
                        {
                            Storage.ImportRootSignature(Id, RootSig);
                        },
                        [&Id, &Storage = m_Context.GetStorage()](const Asset::Handle& RootSig) {} },
                    Desc);
            }
        }
    }

    auto RenderGraphBuilder::LaunchShaderJobs(
        BuildersListType& Builders) const -> void
    {
        for (size_t i = 0; i < Builders.size(); i++)
        {
            auto& Builder = Builders[i];
            m_Passes[i]->ResolveShaders(Builder.Shaders);
        }
        for (auto& Builder : Builders)
        {
            for (auto& [Id, Desc] : Builder.Shaders.m_ShadersToLoad)
            {
                std::visit(
                    VariantVisitor{
                        [&Id, &Storage = m_Context.GetStorage()](const auto& Builder)
                        {
                            Storage.ImportShader(Id, Ptr<RHI::IShader>(RHI::IShader::Create(Builder)));
                        },
                        [&Id, &Storage = m_Context.GetStorage()](const Ptr<RHI::IShader>& Shader)
                        {
                            Storage.ImportShader(Id, Shader);
                        },
                        [&Id, &Storage = m_Context.GetStorage()](const Asset::Handle& Shader) {} },
                    Desc);
            }
        }
    }

    auto RenderGraphBuilder::LaunchPipelineJobs(
        BuildersListType& Builders) -> void
    {
        LaunchShaderJobs(Builders);
        LaunchRootSignatureJobs(Builders);

        for (size_t i = 0; i < Builders.size(); i++)
        {
            auto& Builder = Builders[i];
            m_Passes[i]->ResolvePipelineStates(Builder.PipelineStates);
        }

        for (auto& Builder : Builders)
        {
            for (auto& [Id, Desc] : Builder.PipelineStates.m_PipelinesToLoad)
            {
                std::visit(
                    VariantVisitor{
                        [&Id, &Storage = m_Context.GetStorage()](const auto& Builder)
                        {
                            Storage.ImportPipelineState(Id, RHI::IPipelineState::Create(Builder));
                        },
                        [&Id, &Storage = m_Context.GetStorage()](const Ptr<RHI::IPipelineState>& PipelineState)
                        {
                            Storage.ImportPipelineState(Id, PipelineState);
                        },
                    },
                    Desc);
            }
        }
    }

    auto RenderGraphBuilder::BuildPasses(
        BuildersListType& Builders) -> std::vector<RenderGraphDepdencyLevel>
    {
        BuildAdjacencyLists(Builders);
        TopologicalSort();
        auto ResourcesToDestroy = CalculateResourcesLifetime(Builders);
        return BuildDependencyLevels(Builders, ResourcesToDestroy);
    }

    //

    void RenderGraphBuilder::BuildAdjacencyLists(
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

    void RenderGraphBuilder::TopologicalSort()
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

    auto RenderGraphBuilder::CalculateResourcesLifetime(
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

    void RenderGraphBuilder::DepthFirstSearch(
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

    auto RenderGraphBuilder::BuildDependencyLevels(
        BuildersListType&                  Builders,
        std::vector<std::set<ResourceId>>& ResourceToDestroy) -> std::vector<RenderGraphDepdencyLevel>
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

        std::vector<RenderGraphDepdencyLevel> Dependencies;
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

    IRenderPass& RenderGraphBuilder::AppendPass(
        std::unique_ptr<IRenderPass> Pass)
    {
        return *m_Passes.emplace_back(std::move(Pass));
    }

    RenderGraphBuilder::BuilderInfo::BuilderInfo(
        GraphStorage& Storage) :
        Resources(Storage),
        PipelineStates(Storage)
    {
    }
} // namespace Neon::RG
