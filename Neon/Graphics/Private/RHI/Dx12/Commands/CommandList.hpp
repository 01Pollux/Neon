#pragma once

#include <RHI/Commands/List.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <RHI/Resource/Views/GenericView.hpp>
#include <Private/RHI/Dx12/Resource/Descriptor.hpp>

namespace Neon::RHI
{
    class Dx12Swapchain;

    class Dx12CommandList : public ICommandList
    {
    public:
        void CopySubresources(
            IGpuResource*                    DstResource,
            IGpuResource*                    Intermediate,
            size_t                           IntOffset,
            uint32_t                         FirstSubresource,
            std::span<const SubresourceDesc> SubResources) override;

        void CopyResource(
            IGpuResource* DstResource,
            IGpuResource* SrcResource) override;

        void CopyBufferRegion(
            IBuffer* DstBuffer,
            size_t   DstOffset,
            IBuffer* SrcBuffer,
            size_t   SrcOffset,
            size_t   NumBytes) override;

        void CopyTextureRegion(
            const TextureCopyLocation& Dst,
            const Vector3I&            DstPosition,
            const TextureCopyLocation& Src,
            const CopyBox*             SrcBox = nullptr) override;

    public:
        void SetRootSignature(
            bool                       IsDirect,
            const Ptr<IRootSignature>& RootSig) override;

        void SetPipelineState(
            const Ptr<IPipelineState>& State) override;

    public:
        void SetDynamicResourceView(
            bool                IsDirect,
            CstResourceViewType Type,
            uint32_t            RootIndex,
            const void*         Data,
            size_t              Size) override;

    public:
        void SetConstants(
            bool        IsDirect,
            uint32_t    RootIndex,
            const void* Constants,
            size_t      NumConstants32Bit,
            size_t      DestOffset = 0) override;

        void SetResourceView(
            bool                IsDirect,
            CstResourceViewType Type,
            uint32_t            RootIndex,
            GpuResourceHandle   Handle) override;

        void SetDescriptorTable(
            bool                IsDirect,
            uint32_t            RootIndex,
            GpuDescriptorHandle Handle) override;

    public:
        void ClearRtv(
            CpuDescriptorHandle RtvHandle,
            const Color4&       Color) override;

        void ClearDsv(
            CpuDescriptorHandle    RtvHandle,
            std::optional<float>   Depth,
            std::optional<uint8_t> Stencil) override;

        void SetRenderTargets(
            CpuDescriptorHandle        ContiguousRtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) override;

        void SetRenderTargets(
            const CpuDescriptorHandle* Rtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) override;

    public:
        void SetScissorRect(
            std::span<RectT<Vector2>> Scissors) override;

        void SetViewport(
            std::span<ViewportF> Views) override;

        void SetPrimitiveTopology(
            PrimitiveTopology Topology) override;

        void SetIndexBuffer(
            const Views::Index& View) override;

        void SetVertexBuffer(
            size_t               StartSlot,
            const Views::Vertex& Views) override;

    public:
        void Draw(
            const DrawIndexArgs& Args) override;

        void Draw(
            const DrawArgs& Args) override;

    public:
        void Dispatch(
            size_t GroupCountX = 1,
            size_t GroupCountY = 1,
            size_t GroupCountZ = 1) override;

    public:
        /// <summary>
        /// Reset pipeline state and root signature.
        /// </summary>
        void Reset();

    public:
        /// <summary>
        /// Attach D3D12 command list.
        /// </summary>
        void AttachCommandList(
            ID3D12GraphicsCommandList* CommandList);

        /// <summary>
        /// Get underlying D3D12 command list.
        /// </summary>
        [[nodiscard]] ID3D12GraphicsCommandList* Get();

    private:
        ID3D12GraphicsCommandList* m_CommandList = nullptr;

        Ptr<IPipelineState> m_PipelineState;
        Ptr<IRootSignature> m_RootSignature;
    };
} // namespace Neon::RHI