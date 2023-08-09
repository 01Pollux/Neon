#pragma once

#include <RenderGraph/Common.hpp>

namespace Neon::RHI
{
    class ICommandList;
    class IGraphicsCommandList;
    class IComputeCommandList;
} // namespace Neon::RHI

namespace Neon::RG
{
    class GraphStorage;

    class IRenderPass
    {
    public:
        class ResourceResolver;

        IRenderPass(
            StringU8      PassName,
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
        /// Set render pass flags
        /// </summary>
        void SetPassFlags(
            MPassFlags Flags)
        {
            m_Flags = Flags;
        }

        /// <summary>
        /// Get render pass flags
        /// </summary>
        [[nodiscard]] MPassFlags GetPassFlags() const noexcept
        {
            return m_Flags;
        }

        /// <summary>
        /// Get pass name
        /// </summary>
        [[nodiscard]] const StringU8& GetPassName() const noexcept
        {
            return m_PassName;
        }

        /// <summary>
        /// Get queue type
        /// </summary>
        [[nodiscard]] PassQueueType GetQueueType() const noexcept
        {
            return m_QueueType;
        }

    private:
        StringU8      m_PassName;
        PassQueueType m_QueueType;
        MPassFlags    m_Flags;
    };

    using ResourceResolver = IRenderPass::ResourceResolver;
} // namespace Neon::RG