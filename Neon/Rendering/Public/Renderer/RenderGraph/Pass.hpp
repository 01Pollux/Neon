#pragma once

#include <Renderer/RenderGraph/ResourceId.hpp>
#include <Math/Size2.hpp>

namespace Renderer::RG
{
    class RenderGraphPassResBuilder;
    class RenderGraphPassRSBuilder;
    class RenderGraphPassShaderBuilder;
    class RenderGraphPassPSOBuilder;

    class RenderGraphStorage;

    class IRenderPass
    {
    public:
        IRenderPass(
            PassQueueType QueueType) noexcept;

        virtual ~IRenderPass() = default;

        /// <summary>
        /// Setup render pass' resources
        /// </summary>
        virtual void SetupResources(
            RenderGraphPassResBuilder& PassBuilder) = 0;

        /// <summary>
        /// Setup render pass' root signatures
        /// </summary>
        virtual void SetupRootSignatures(
            RenderGraphPassRSBuilder& PassBuilder) = 0;

        /// <summary>
        /// Setup render pass' root signatures
        /// </summary>
        virtual void SetupShaders(
            RenderGraphPassShaderBuilder& PassBuilder) = 0;

        /// <summary>
        /// Setup render pass' root pipeline states
        /// </summary>
        virtual void SetupPipelineStates(
            RenderGraphPassPSOBuilder& PassBuilder) = 0;

        /// <summary>
        /// Called to check if the pass should implements its own viewports
        /// </summary>
        virtual bool OverrideViewport(
            const RenderGraphStorage&    GraphStorage,
            RHI::GraphicsCommandContext& CmdContext);

        /// <summary>
        /// insert commands into the command list to be later executed
        /// </summary>
        virtual void Execute(
            const RenderGraphStorage& GraphStorage,
            RHI::ICommandContext&     CmdContext) = 0;

    public:
        /// <summary>
        /// Get queue type
        /// </summary>
        [[nodiscard]] PassQueueType GetQueueType() const;

        /// <summary>
        /// Get viewport for current pass
        /// </summary>
        [[nodiscard]] const Size2I& GetViewport() const;

        /// <summary>
        /// Set viewport for current pass
        /// </summary>
        void SetViewport(const Size2I& Viewport);

        /// <summary>
        /// Set viewport to window's viewport for current pass
        /// </summary>
        void SetWindowViewport();

    private:
        Size2I        m_Viewport;
        PassQueueType m_QueueType;
    };
} // namespace Renderer::RG