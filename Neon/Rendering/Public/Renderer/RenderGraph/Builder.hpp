#pragma once

#include <Renderer/RenderGraph/Context.hpp>
#include <Renderer/RenderGraph/PassBuilder.hpp>
#include <Renderer/RenderGraph/Pass.hpp>
#include <Utils/LockedData.hpp>
#include <future>

namespace Renderer::RG
{
    class RenderGraphBuilder
    {
        friend class RenderGraphContext;

        struct BuilderInfo
        {
            RenderGraphPassResBuilder    Resources;
            RenderGraphPassRSBuilder     RootSignatures;
            RenderGraphPassShaderBuilder Shaders;
            RenderGraphPassPSOBuilder    PipelineStates;

            BuilderInfo(
                RenderGraphStorage&  Storage,
                const ShaderMapType& LoadedShaders);
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
            IRenderPass::UPtr Pass);

        /// <summary>
        /// Append render pass
        /// </summary>
        template<typename _Ty, typename... _Args>
        _Ty& AppendPass(
            _Args&&... Args)
        {
            return static_cast<_Ty&>(AppendPass(std::make_unique<_Ty>(std::forward<_Args>(Args)...)));
        }

        /// <summary>
        /// Get backbuffer format
        /// </summary>
        [[nodiscard]] static DXGI_FORMAT GetBackbufferFormat();

    private:
        [[nodiscard]] std::vector<std::future<void>> LaunchRootSignatureJobs(
            const std::shared_ptr<BuildersListType>& Builders);

        [[nodiscard]] std::vector<std::future<void>> LaunchShaderJobs(
            const std::shared_ptr<BuildersListType>&            Builders,
            const std::shared_ptr<LockableData<ShaderMapType>>& LoadedShaders) const;

        [[nodiscard]] std::future<void> LaunchPipelineJobs(
            const std::shared_ptr<BuildersListType>&            Builders,
            const std::shared_ptr<LockableData<ShaderMapType>>& LoadedShaders);

    private:
        [[nodiscard]] std::vector<RenderGraphContext::DepdencyLevel> BuildPasses(
            const std::shared_ptr<BuildersListType>& Builders);

        void BuildAdjacencyLists(
            const BuildersListType& Builders);

        void TopologicalSort();

        [[nodiscard]] std::vector<std::set<ResourceId>>
        CalculateResourcesLifetime(
            BuildersListType& Builders) const;

        void DepthFirstSearch(
            size_t              Index,
            std::vector<bool>&  Visited,
            std::stack<size_t>& Stack);

        [[nodiscard]] std::vector<RenderGraphContext::DepdencyLevel>
        BuildDependencyLevels(
            BuildersListType&                  Builders,
            std::vector<std::set<ResourceId>>& ResourceToDestroy);

    private:
        explicit RenderGraphBuilder(
            RenderGraphContext& Context);

    private:
        RenderGraphContext& m_Context;

        std::vector<IRenderPass::UPtr> m_Passes;

        AdjacencyListType   m_AdjacencyList;
        std::vector<size_t> m_TopologicallySortedList;
    };
} // namespace Renderer::RG