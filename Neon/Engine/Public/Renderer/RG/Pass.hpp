#pragma once

#include <Renderer/RG/Common.hpp>

namespace Neon::RHI
{
    class ICommandList;
    class IGraphicsCommandList;
} // namespace Neon::RHI

namespace Neon::RG
{
    class GraphStorage;

    class IRenderPass
    {
    public:
        class ShaderResolver;
        class RootSignatureResolver;
        class PipelineStateResolver;
        class ResourceResolver;

        IRenderPass(
            PassQueueType QueueType) :
            m_QueueType(QueueType)
        {
        }

        virtual ~IRenderPass() = default;

        /// <summary>
        /// Called when the render pass wans to load shaders.
        /// </summary>
        virtual void ResolveShaders(
            ShaderResolver& Resolver)
        {
        }

        /// <summary>
        /// Called when the render pass wans to load shaders.
        /// </summary>
        virtual void ResolveRootSignature(
            RootSignatureResolver& Resolver)
        {
        }

        /// <summary>
        /// Called when the render pass wants to create pipelines.
        /// </summary>
        virtual void ResolvePipelineStates(
            PipelineStateResolver& Resolver)
        {
        }

        /// <summary>
        /// Called when the render pass wants to resolve the dependencies of resources.
        /// </summary>
        virtual void ResolveResources(
            ResourceResolver& Resolver)
        {
        }

        /// <summary>
        /// Called to check if the pass should implements its own viewports
        /// </summary>
        virtual bool OverrideViewport(
            const GraphStorage&        Storage,
            RHI::IGraphicsCommandList* CommandList)
        {
            return false;
        }

        /// <summary>
        /// Called when the render pass wants to dispatch.
        /// </summary>
        virtual void Dispatch(
            const GraphStorage& Storage,
            RHI::ICommandList*  CommandList)
        {
        }

    public:
        /// <summary>
        /// Get queue type
        /// </summary>
        [[nodiscard]] PassQueueType GetQueueType() const noexcept
        {
            return m_QueueType;
        }

    private:
        PassQueueType m_QueueType;
    };
} // namespace Neon::RG