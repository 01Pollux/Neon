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
        class ResourceResolver;

        IRenderPass(
            PassQueueType QueueType) :
            m_QueueType(QueueType)
        {
        }

        virtual ~IRenderPass() = default;

        /// <summary>
        /// Called when the render pass wants to resolve the dependencies of resources.
        /// </summary>
        virtual void ResolveResources(
            ResourceResolver&)
        {
        }

        /// <summary>
        /// Called to check if the pass should implements its own viewports
        /// </summary>
        virtual bool OverrideViewport(
            const GraphStorage&,
            RHI::IGraphicsCommandList*)
        {
            return false;
        }

        /// <summary>
        /// Called when the render pass wants to dispatch.
        /// </summary>
        virtual void Dispatch(
            const GraphStorage&,
            RHI::ICommandList*)
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