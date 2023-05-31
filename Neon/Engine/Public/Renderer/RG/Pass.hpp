#pragma once

#include <Renderer/RG/Common.hpp>

namespace Neon::RHI
{
    class ICommandContext;
}

namespace Neon::RG
{
    class GraphStorage;

    class IRenderPass
    {
    public:
        class ShaderResolver;
        class MaterialResolver;
        class ResourceResolver;
        class PipelineResolver;

        IRenderPass(
            PassQueueType QueueType);

        virtual ~IRenderPass() = default;

        /// <summary>
        /// Called when the render pass wants to resolve the dependencies of resources.
        /// </summary>
        virtual void ResolveResources(
            ResourceResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wans to load shaders.
        /// </summary>
        virtual void ResolveShaders(
            ShaderResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wants to load materials.
        /// </summary>
        virtual void ResolveMaterials(
            MaterialResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wants to create pipelines.
        /// </summary>
        virtual void ResolvePipelines(
            PipelineResolver& Resolver) = 0;

        /// <summary>
        /// Called when the render pass wants to dispatch.
        /// </summary>
        virtual void Dispatch(
            const GraphStorage&   Storage,
            RHI::ICommandContext& CommandContext) = 0;

    public:
        /// <summary>
        /// Get queue type
        /// </summary>
        [[nodiscard]] PassQueueType GetQueueType() const noexcept;

        /// <summary>
        /// Get viewport for current pass
        /// </summary>
        [[nodiscard]] const Size2I& GetViewport() const;

        /// <summary>
        /// Set viewport for current pass
        /// </summary>
        void SetViewport(
            const Size2I& Viewport);

        /// <summary>
        /// Set viewport to window's viewport for current pass
        /// </summary>
        void SetWindowViewport();

    private:
        Size2I        m_Viewport;
        PassQueueType m_QueueType;
    };
} // namespace Neon::RG