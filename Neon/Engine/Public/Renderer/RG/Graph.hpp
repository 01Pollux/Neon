#pragma once

#include <Renderer/RG/Storage.hpp>
#include <future>

namespace Neon::RG
{
    class RenderGraph
    {
        class DepdencyLevel;
        using DepdencyLevelList = std::vector<DepdencyLevel>;

    public:
        class Builder;

        RenderGraph(
            RHI::ISwapchain* Swapchain);

        /// <summary>
        /// Reset resource graph for recording
        /// </summary>
        [[nodiscard]] Builder Reset();

        /// <summary>
        /// Get the storage of the graph
        /// </summary>
        [[nodiscard]] GraphStorage& GetStorage();

        /// <summary>
        /// Get the storage of the graph
        /// </summary>
        [[nodiscard]] const GraphStorage& GetStorage() const;

        /// <summary>
        /// Run the graph
        /// </summary>
        void Run();

    private:
        /// <summary>
        /// Build levels and imported resources
        /// </summary>
        void Build(
            std::vector<DepdencyLevel>&& Levels);

    private:
        GraphStorage      m_Storage;
        DepdencyLevelList m_Levels;
        std::future<void> m_PipelineCreators;
    };

    //

    class RenderGraph::DepdencyLevel
    {
    public:
        DepdencyLevel(
            RenderGraph& Context);

        DepdencyLevel(const DepdencyLevel&)            = delete;
        DepdencyLevel(DepdencyLevel&&)                 = default;
        DepdencyLevel& operator=(const DepdencyLevel&) = delete;
        DepdencyLevel& operator=(DepdencyLevel&&)      = default;
        ~DepdencyLevel()                               = default;

        /// <summary>
        /// Append render pass
        /// </summary>
        void AddPass(
            IRenderPass::UPtr                               Pass,
            std::vector<ResourceViewId>                     RenderTargets,
            std::optional<ResourceViewId>                   DepthStencil,
            std::set<ResourceId>                            ResourceToCreate,
            std::set<ResourceId>                            ResourceToDestroy,
            std::map<ResourceViewId, D3D12_RESOURCE_STATES> States);

        /// <summary>
        /// Execute render passes
        /// </summary>
        void Execute(
            RHI::IDisplayBuffers* Display);

    private:
        /// <summary>
        /// Execute pending resource barriers before render passes
        /// </summary>
        void ExecuteBarriers(
            RHI::IDisplayBuffers* Display);

        /// <summary>
        /// Execute render passes
        /// </summary>
        void ExecutePasses(
            RHI::IDisplayBuffers* Display) const;

    private:
        struct RenderPassInfo
        {
            IRenderPass::UPtr             Pass;
            std::vector<ResourceViewId>   RenderTargets;
            std::optional<ResourceViewId> DepthStencil;
        };

        RenderGraphContext& m_Context;

        std::vector<RenderPassInfo> m_Passes;

        std::set<ResourceId> m_ResourcesToCreate;
        std::set<ResourceId> m_ResourcesToDestroy;

        std::map<ResourceViewId, D3D12_RESOURCE_STATES> m_States;
    };
} // namespace Neon::RG