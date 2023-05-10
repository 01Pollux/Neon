#pragma once

#include <RHI/Commands/Commands.hpp>

namespace Neon
{
    class Color4;
}

namespace Neon::RHI
{
    class ICommandAllocator;
    struct CpuDescriptorHandle;
    struct GpuDescriptorHandle;

    class ICommandList
    {
    public:
        [[nodiscard]] static ICommandList* Create(
            ICommandAllocator* Allocator,
            CommandQueueType   Type);

        virtual ~ICommandList() = default;

        /// <summary>
        /// Reset command list.
        /// </summary>
        virtual void Reset(
            ICommandAllocator* Allocator) = 0;

        /// <summary>
        /// Close command list.
        /// </summary>
        virtual void Close() = 0;
    };

    //

    class ICommonCommandList : public virtual ICommandList
    {
    public:
    };

    class IGraphicsCommandList : public virtual ICommonCommandList
    {
    public:
        /// <summary>
        /// Clear render target view
        /// </summary>
        virtual void ClearRtv(
            const CpuDescriptorHandle& RtvHandle,
            const Color4&              Color) = 0;

        /// <summary>
        /// Bind rtv/dsv to pipeline
        /// </summary>
        virtual void SetRenderTargets(
            const CpuDescriptorHandle& ContiguousRtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) = 0;

        /// <summary>
        /// Bind rtv/dsv to pipeline
        /// </summary>
        virtual void SetRenderTargets(
            const CpuDescriptorHandle* Rtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) = 0;
    };
} // namespace Neon::RHI
