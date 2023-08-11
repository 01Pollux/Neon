#pragma once

#include <RenderGraph/Storage.hpp>
#include <RenderGraph/Pass.hpp>
#include <RHI/Commands/Context.hpp>

#include <Asio/ThreadPool.hpp>

namespace Neon::RG
{
    class IRenderPass;
    class GraphBuilder;
    class GraphDepdencyLevel;

    class RenderGraph
    {
        friend class GraphDepdencyLevel;

        friend class GraphBuilder;
        using DepdencyLevelList = std::vector<GraphDepdencyLevel>;

        using RenderCommandContext  = RHI::TCommandContext<RHI::CommandQueueType::Graphics>;
        using ComputeCommandContext = RHI::TCommandContext<RHI::CommandQueueType::Compute>;

    public:
        /// <summary>
        /// Reset resource graph for recording
        /// </summary>
        [[nodiscard]] GraphBuilder Reset();

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
        void Run(
            RHI::GpuResourceHandle CameraBuffer,
            bool                   CopyToBackBuffer);

    private:
        /// <summary>
        /// Build levels and imported resources
        /// </summary>
        void Build(
            DepdencyLevelList&& Levels);

        /// <summary>
        /// Submit to back buffer by copying the output image aswell as the debug overlay
        /// </summary>
        void SubmitToBackBuffer(
            RHI::ICommonCommandList* CommandList,
            RHI::GpuResourceHandle   CameraBuffer);

    private:
        GraphStorage      m_Storage;
        DepdencyLevelList m_Levels;

        std::mutex         m_RenderMutex, m_ComputeMutex;
        Asio::ThreadPool<> m_ThreadPool{ 3 };
    };

    //

    class GraphDepdencyLevel
    {

    public:
        GraphDepdencyLevel(
            RenderGraph& Context);

        GraphDepdencyLevel(const GraphDepdencyLevel&)            = delete;
        GraphDepdencyLevel(GraphDepdencyLevel&&)                 = default;
        GraphDepdencyLevel& operator=(const GraphDepdencyLevel&) = delete;
        GraphDepdencyLevel& operator=(GraphDepdencyLevel&&)      = default;
        ~GraphDepdencyLevel()                                    = default;

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
            RenderGraph::RenderCommandContext&  RenderContext,
            RenderGraph::ComputeCommandContext& ComputeContext) const;

    private:
        /// <summary>
        /// Execute pending resource barriers before render passes
        /// </summary>
        void ExecuteBarriers(
            RenderGraph::RenderCommandContext&  RenderContext,
            RenderGraph::ComputeCommandContext& ComputeContext) const;

        /// <summary>
        /// Execute render passes
        /// </summary>
        void ExecutePasses(
            RenderGraph::RenderCommandContext&  RenderContext,
            RenderGraph::ComputeCommandContext& ComputeContext) const;

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