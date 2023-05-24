#pragma once

#include <RHI/Commands/Commands.hpp>

namespace Neon
{
    class Color4;
    class Vector3DI;
} // namespace Neon

namespace Neon::RHI
{
    class ISwapchain;

    struct CpuDescriptorHandle;
    struct GpuDescriptorHandle;

    class IPipelineState;
    class IRootSignature;

    class IGpuResource;
    class ITexture;
    class IBuffer;

    struct SubresourceDesc;
    struct TextureCopyLocation;

    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;

        /// <summary>
        /// Copy subresources into the buffer
        /// </summary>
        virtual void CopySubresources(
            IGpuResource*              DstResource,
            IGpuResource*              Intermediate,
            size_t                     IntOffset,
            uint32_t                   FirstSubresource,
            std::span<SubresourceDesc> SubResources) = 0;

        /// <summary>
        /// Copy Resource into the buffer
        /// </summary>
        virtual void CopyResources(
            IGpuResource* DstResource,
            IGpuResource* SrcResource) = 0;

        /// <summary>
        /// Copy buffer resource
        /// </summary>
        virtual void CopyBufferRegion(
            IBuffer* DstBuffer,
            size_t   DstOffset,
            IBuffer* SrcBuffer,
            size_t   SrcOffset,
            size_t   NumBytes) = 0;

        struct CopyBox
        {
            uint32_t Left;
            uint32_t Top;
            uint32_t Front;
            uint32_t Right;
            uint32_t Bottom;
            uint32_t Back;
        };

        /// <summary>
        /// Copy texture resource
        /// </summary>
        virtual void CopyTextureRegion(
            const TextureCopyLocation& Dst,
            const Vector3DI&           DstPosition,
            const TextureCopyLocation& Src,
            const CopyBox*             SrcBox = nullptr) = 0;
    };

    //

    class ICommonCommandList : public virtual ICommandList
    {
    public:
        /// <summary>
        /// Set root signature
        /// </summary>
        virtual void SetRootSignature(
            IRootSignature* RootSig) = 0;

        /// <summary>
        /// Set pipeline state
        /// </summary>
        virtual void SetPipelineState(
            IPipelineState* State) = 0;
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
