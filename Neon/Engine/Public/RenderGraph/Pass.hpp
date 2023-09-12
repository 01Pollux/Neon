#pragma once

#include <RenderGraph/Common.hpp>
#include <RHI/Commands/List.hpp>

namespace Neon::RG
{
    class GraphStorage;

    class IRenderPass
    {
    public:
        class ResourceResolver;

        IRenderPass(
            StringU8      PassName,
            PassQueueType QueueType,
            MPassFlags    Flags = {}) :
#ifndef NEON_DIST
            m_PassName(std::move(PassName)),
#endif
            m_QueueType(QueueType),
            m_Flags(std::move(Flags))
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
            RHI::ICommandList*)
        {
            return false;
        }

        /// <summary>
        /// Called before executing the barriers for passes.
        /// </summary>
        virtual void PreDispatch(
            const GraphStorage&)
        {
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
        /// Get pass name
        /// </summary>
        [[nodiscard]] const StringU8& GetPassName() const noexcept
        {
#ifndef NEON_DIST
            return m_PassName;
#else
            return StringUtils::Empty<StringU8>;
#endif
        }

        /// <summary>
        /// Get queue type
        /// </summary>
        [[nodiscard]] PassQueueType GetQueueType() const noexcept
        {
            return m_QueueType;
        }

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

    private:
#ifndef NEON_DIST
        StringU8 m_PassName;
#endif

        PassQueueType m_QueueType;
        MPassFlags    m_Flags;
    };

    using ResourceResolver = IRenderPass::ResourceResolver;

    //

    template<typename _Ty, PassQueueType _Type>
    class TypedRenderPass : public IRenderPass
    {
    public:
        using RenderPass = TypedRenderPass<_Ty, _Type>;

        TypedRenderPass(
            StringU8   PassName,
            MPassFlags Flags = {}) :
            IRenderPass(std::move(PassName), _Type, std::move(Flags))
        {
        }

        void Dispatch(
            const GraphStorage& Graph,
            RHI::ICommandList*  CommandList) final
        {
            if constexpr (_Type == PassQueueType::Unknown)
            {
                static_cast<_Ty*>(this)->DispatchTyped(Graph);
            }
            else if constexpr (_Type == PassQueueType::Direct)
            {
                static_cast<_Ty*>(this)->DispatchTyped(Graph, RHI::GraphicsCommandList(CommandList));
            }
            else if constexpr (_Type == PassQueueType::Compute)
            {
                static_cast<_Ty*>(this)->DispatchTyped(Graph, RHI::ComputeCommandList(CommandList));
            }
        }
    };

    template<typename _Ty>
    using GenericRenderPass = TypedRenderPass<_Ty, PassQueueType::Unknown>;
    template<typename _Ty>
    using GraphicsRenderPass = TypedRenderPass<_Ty, PassQueueType::Direct>;
    template<typename _Ty>
    using ComputeRenderPass = TypedRenderPass<_Ty, PassQueueType::Compute>;
} // namespace Neon::RG