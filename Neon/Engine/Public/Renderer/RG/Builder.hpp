#pragma once

#include <Renderer/RG/Resolver.hpp>
#include <vector>
#include <future>
#include <stack>

namespace Neon::RG
{
    class RenderGraphDepdencyLevel;

    class RenderGraphBuilder
    {
        friend class RenderGraph;

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

        using BuildersListType  = std::vector<BuilderInfo>;
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
        /*/// <summary>
        /// Build root signatures from builders
        /// </summary>
        [[nodiscard]] auto LaunchRootSignatureJobs(
            const std::shared_ptr<BuildersListType>& Builders) -> std::vector<std::jthread>;

        /// <summary>
        /// Build shaders from builders
        /// </summary>
        [[nodiscard]] auto LaunchShaderJobs(
            const std::shared_ptr<BuildersListType>& Builders) const -> std::future<LoadedShaderResult>;

        /// <summary>
        /// Build pipeline states from builders
        /// </summary>
        [[nodiscard]] auto LaunchPipelineJobs(
            const std::shared_ptr<BuildersListType>& Builders) -> std::jthread;

        /// <summary>
        /// Create a copy of render passes
        /// </summary>
        [[nodiscard]] auto CopyRenderPasses() const -> std::vector<IRenderPass*>;*/

        /// <summary>
        /// Build root signatures from builders
        /// </summary>
        [[nodiscard]] auto LaunchRootSignatureJobs(
            BuildersListType& Builders) -> void;

        /// <summary>
        /// Build shaders from builders
        /// </summary>
        [[nodiscard]] auto LaunchShaderJobs(
            BuildersListType& Builders) const -> void;

        /// <summary>
        /// Build pipeline states from builders
        /// </summary>
        [[nodiscard]] auto LaunchPipelineJobs(
            BuildersListType& Builders) -> void;

    private:
        /// <summary>
        /// Build passes from builders
        /// </summary>
        [[nodiscard]] auto BuildPasses(
            BuildersListType& Builders) -> std::vector<RenderGraphDepdencyLevel>;

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
            std::vector<std::set<ResourceId>>& ResourceToDestroy) -> std::vector<RenderGraphDepdencyLevel>;

    private:
        explicit RenderGraphBuilder(
            RenderGraph& Context);

    private:
        RenderGraph& m_Context;

        std::vector<UPtr<IRenderPass>> m_Passes;

        AdjacencyListType   m_AdjacencyList;
        std::vector<size_t> m_TopologicallySortedList;
    };
} // namespace Neon::RG