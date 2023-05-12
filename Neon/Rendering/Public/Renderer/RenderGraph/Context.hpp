#pragma once

#include <Renderer/RenderGraph/Storage.hpp>
#include <Renderer/RenderGraph/Pass.hpp>

#include <stack>
#include <future>

namespace RHI
{
    class IDisplayBuffers;
}

namespace Renderer::RG
{
    class RenderGraphBuilder;
    class RenderGraphStorage;

    class IRenderPass;

    class RenderGraphContext
    {
        NEON_DECLARE_BASECLASS(RenderGraphContext);

        friend class RenderGraphBuilder;
        class DepdencyLevel;

    public:
        RenderGraphContext();

        explicit RenderGraphContext(
            RHI::IDisplayBuffers* Display);

        /// <summary>
        /// Reset render graph context
        /// </summary>
        [[nodiscard]] RenderGraphBuilder Reset();

        /// <summary>
        /// Get render graph storage
        /// </summary>
        [[nodiscard]] RenderGraphStorage& GetStorage();

        /// <summary>
        /// Get render graph storage
        /// </summary>
        [[nodiscard]] const RenderGraphStorage& GetStorage() const;

    public:
        /// <summary>
        /// Execute render graph's nodes
        /// </summary>
        void Execute();

    private:
        /// <summary>
        /// Build levels and imported resources
        /// </summary>
        void Build(
            std::vector<DepdencyLevel>&& Levels);

    private:
        RenderGraphStorage         m_Storage;
        std::vector<DepdencyLevel> m_Levels;
        std::future<void>          m_PipelineCreators;
    };

    class RenderGraphContext::DepdencyLevel
    {
    public:
        DepdencyLevel(
            RenderGraphContext& Context);

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
} // namespace Renderer::RG
