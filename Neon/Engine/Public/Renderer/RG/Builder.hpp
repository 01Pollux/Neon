#pragma once

#include <Renderer/RG/Graph.hpp>
#include <Renderer/RG/Resolver.hpp>
#include <vector>
#include <future>
#include <stack>

namespace Neon::RG
{
    class RenderGraph::Builder
    {
        friend class RenderGraphContext;

        struct BuilderInfo
        {
            IRenderPass::ShaderResolver        Shaders;
            IRenderPass::RootSignatureResolver RootSignatures;
            IRenderPass::PipelineStateResolver PipelineStates;
            IRenderPass::MaterialResolver      Material;
            IRenderPass::ResourceResolver      Resources;

            BuilderInfo(
                GraphStorage& Storage);
        };

        using BuildersListType  = std::list<BuilderInfo>;
        using AdjacencyListType = std::vector<std::vector<size_t>>;

    public:
        /// <summary>
        /// Build render graph from graph builder
        /// </summary>
        void Build();

        /// <summary>
        /// Append render pass
        /// </summary>
        IRenderPass& AppendPass(
            UPtr<IRenderPass> Pass);

        /// <summary>
        /// Append render pass
        /// </summary>
        template<typename _Ty, typename... _Args>
        _Ty& AppendPass(
            _Args&&... Args)
        {
            return static_cast<_Ty&>(AppendPass(std::make_unique<_Ty>(std::forward<_Args>(Args)...)));
        }

    private:
        /// <summary>
        /// Build passes from builders
        /// </summary>
        [[nodiscard]] std::vector<RenderGraph::DepdencyLevel> BuildPasses(
            const std::shared_ptr<BuildersListType>& Builders);

        /// <summary>
        /// Build adjacency lists for passes dependencies
        /// </summary>
        void BuildAdjacencyLists(
            const BuildersListType& Builders);

        /// <summary>
        /// Topological sort of the graph
        /// </summary>
        void TopologicalSort();

        /// <summary>
        /// Calculate resources lifetime for each pass
        /// </summary>
        [[nodiscard]] std::vector<std::set<ResourceId>> CalculateResourcesLifetime(
            BuildersListType& Builders) const;

        /// <summary>
        /// Depth first search for topological sort
        /// </summary>
        void DepthFirstSearch(
            size_t              Index,
            std::vector<bool>&  Visited,
            std::stack<size_t>& Stack);

        /// <summary>
        /// Build dependency levels
        /// </summary>
        [[nodiscard]] std::vector<RenderGraph::DepdencyLevel> BuildDependencyLevels(
            BuildersListType&                  Builders,
            std::vector<std::set<ResourceId>>& ResourceToDestroy);

    private:
        explicit Builder(
            RenderGraph& Context);

    private:
        RenderGraph& m_Context;

        std::vector<UPtr<IRenderPass>> m_Passes;

        AdjacencyListType   m_AdjacencyList;
        std::vector<size_t> m_TopologicallySortedList;
    };
} // namespace Neon::RG