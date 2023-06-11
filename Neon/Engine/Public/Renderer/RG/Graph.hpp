#pragma once

#include <Renderer/RG/Storage.hpp>

namespace Neon::RG
{
    class IRenderPass;
    class RenderGraphBuilder;
    class RenderGraphDepdencyLevel;

    class RenderGraph
    {
        friend class RenderGraphBuilder;
        using DepdencyLevelList = std::vector<RenderGraphDepdencyLevel>;

    public:
        RenderGraph(
            RHI::ISwapchain* Swapchain);

        /// <summary>
        /// Reset resource graph for recording
        /// </summary>
        [[nodiscard]] RenderGraphBuilder Reset();

        /// <summary>
        /// Get the storage of the graph
        /// </summary>
        [[nodiscard]] GraphStorage& GetStorage() noexcept;

        /// <summary>
        /// Get the storage of the graph
        /// </summary>
        [[nodiscard]] const GraphStorage& GetStorage() const noexcept;

        /// <summary>
        /// Run the graph
        /// </summary>
        void Run();

    private:
        /// <summary>
        /// Build levels and imported resources
        /// </summary>
        void Build(
            DepdencyLevelList&& Levels);

    private:
        GraphStorage      m_Storage;
        DepdencyLevelList m_Levels;
    };

    //

    class RenderGraphDepdencyLevel
    {
    public:
        RenderGraphDepdencyLevel(
            RenderGraph& Context);

        RenderGraphDepdencyLevel(const RenderGraphDepdencyLevel&)            = delete;
        RenderGraphDepdencyLevel(RenderGraphDepdencyLevel&&)                 = default;
        RenderGraphDepdencyLevel& operator=(const RenderGraphDepdencyLevel&) = delete;
        RenderGraphDepdencyLevel& operator=(RenderGraphDepdencyLevel&&)      = default;
        ~RenderGraphDepdencyLevel()                                          = default;

        /// <summary>
        /// Append render pass
        /// </summary>
        void AddPass(
            UPtr<IRenderPass>                             Pass,
            std::vector<ResourceViewId>                   RenderTargets,
            std::optional<ResourceViewId>                 DepthStencil,
            std::set<ResourceId>                          ResourceToCreate,
            std::set<ResourceId>                          ResourceToDestroy,
            std::map<ResourceViewId, RHI::MResourceState> States);

        /// <summary>
        /// Execute render passes
        /// </summary>
        void Execute(
            RHI::ISwapchain* Swapchain);

    private:
        /// <summary>
        /// Execute pending resource barriers before render passes
        /// </summary>
        void ExecuteBarriers(
            RHI::ISwapchain* Swapchain);

        /// <summary>
        /// Execute render passes
        /// </summary>
        void ExecutePasses(
            RHI::ISwapchain* Swapchain) const;

    private:
        struct RenderPassInfo
        {
            UPtr<IRenderPass>             Pass;
            std::vector<ResourceViewId>   RenderTargets;
            std::optional<ResourceViewId> DepthStencil;
        };

        RenderGraph& m_Context;

        std::vector<RenderPassInfo> m_Passes;

        std::set<ResourceId> m_ResourcesToCreate;
        std::set<ResourceId> m_ResourcesToDestroy;

        std::map<ResourceViewId, RHI::MResourceState> m_States;
    };
} // namespace Neon::RG