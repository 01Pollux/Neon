#pragma once

#include <Renderer/RG/Storage.hpp>
#include <Asio/ThreadPool.hpp>

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
        GraphStorage       m_Storage;
        DepdencyLevelList  m_Levels;
        Asio::ThreadPool<> m_ThreadPool{ 4 };
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
            Asio::ThreadPool<>& ThreadPool);

    private:
        /// <summary>
        /// Execute pending resource barriers before render passes
        /// </summary>
        void ExecuteBarriers();

        /// <summary>
        /// Execute render passes
        /// </summary>
        void ExecutePasses(
            Asio::ThreadPool<>& ThreadPool) const;

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