#pragma once

#include <RHI/Commands/Commands.hpp>

namespace Neon
{
    class Color4;
}

namespace Neon::RHI
{
    class ISwapchain;

    struct CpuDescriptorHandle;
    struct GpuDescriptorHandle;

    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;
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
