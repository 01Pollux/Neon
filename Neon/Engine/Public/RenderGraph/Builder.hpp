#pragma once

#include <RenderGraph/Resolver.hpp>
#include <vector>
#include <future>
#include <stack>

namespace Neon::RG
{
    class GraphDepdencyLevel;

    class GraphBuilder
    {
        friend class RenderGraph;

        struct BuilderInfo
        {
            IRenderPass::ResourceResolver Resources;
            BuilderInfo(
                GraphStorage& Storage);
        };

        using BuildersListType  = std::vector<BuilderInfo>;
        using AdjacencyListType = std::vector<std::vector<size_t>>;

    public:
        /// <summary>
        /// Build render graph from graph builder
        /// </summary>
        void Build(
            ResourceId FinalOutput);

        /// <summary>
        /// Append render pass
        /// </summary>
        IRenderPass& AddPass(
            UPtr<IRenderPass> Pass);

        /// <summary>
        /// Append render pass
        /// </summary>
        template<typename _Ty, typename... _Args>
        _Ty& AddPass(
            _Args&&... Args)
        {
            return static_cast<_Ty&>(AddPass(std::make_unique<_Ty>(std::forward<_Args>(Args)...)));
        }

    private:
        /// <summary>
        /// Build passes from builders
        /// </summary>
        [[nodiscard]] auto BuildPasses(
            BuildersListType& Builders) -> std::vector<GraphDepdencyLevel>;

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
        [[nodiscard]] auto CalculateResourcesLifetime(
            BuildersListType& Builders) const -> std::vector<std::set<ResourceId>>;

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
        [[nodiscard]] auto BuildDependencyLevels(
            BuildersListType&                  Builders,
            std::vector<std::set<ResourceId>>& ResourceToDestroy) -> std::vector<GraphDepdencyLevel>;

    private:
        explicit GraphBuilder(
            RenderGraph& Context);

    private:
        RenderGraph& m_Context;

        std::vector<UPtr<IRenderPass>> m_Passes;

        AdjacencyListType   m_AdjacencyList;
        std::vector<size_t> m_TopologicallySortedList;
    };
} // namespace Neon::RG