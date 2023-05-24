#pragma once

#include <RHI/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>

namespace Neon::RHI
{
    class Dx12Swapchain;

    class Dx12CommandList : public virtual ICommandList
    {
    public:
        Dx12CommandList() = default;
        NEON_CLASS_NO_COPYMOVE(Dx12CommandList);
        ~Dx12CommandList() override;

        void CopySubresources(
            IGpuResource*              DstResource,
            IGpuResource*              Intermediate,
            size_t                     IntOffset,
            uint32_t                   FirstSubresource,
            std::span<SubresourceDesc> SubResources) override;

        void CopyResources(
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
            const Vector3DI&           DstPosition,
            const TextureCopyLocation& Src,
            const CopyBox*             SrcBox = nullptr) override;

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

    protected:
        ID3D12GraphicsCommandList* m_CommandList = nullptr;
    };

    class Dx12CommonCommandList : public virtual ICommonCommandList,
                                  public Dx12CommandList

    {
    public:
        using Dx12CommandList::Dx12CommandList;

        void SetPipelineState(
            IPipelineState* State) override;
    };

    class Dx12GraphicsCommandList final : public virtual IGraphicsCommandList,
                                          public Dx12CommonCommandList
    {
    public:
        using Dx12CommonCommandList::Dx12CommonCommandList;

        void SetRootSignature(
            IRootSignature* RootSig) override;

        void ClearRtv(
            const CpuDescriptorHandle& RtvHandle,
            const Color4&              Color) override;

        void SetRenderTargets(
            const CpuDescriptorHandle& ContiguousRtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) override;

        void SetRenderTargets(
            const CpuDescriptorHandle* Rtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) override;

    public:
        void SetConstants(
            uint32_t    RootIndex,
            const void* Constants,
            size_t      NumConstants32Bit,
            size_t      DestOffset = 0) override;

        void SetResourceView(
            ViewType Type,
            uint32_t RootIndex,
            IBuffer* Resource) override;

        void SetDescriptorTable(
            uint32_t            RootIndex,
            GpuDescriptorHandle Handle) override;

    public:
        void SetScissorRect(
            std::span<RectT<Vector2D>> Scissors) override;

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
    };

    //

    class Dx12ComputeCommandList final : public virtual IComputeCommandList,
                                         public Dx12CommonCommandList
    {
    public:
        void SetRootSignature(
            IRootSignature* RootSig) override;

    public:
        void SetConstants(
            uint32_t    RootIndex,
            const void* Constants,
            size_t      NumConstants32Bit,
            size_t      DestOffset = 0) override;

        void SetResourceView(
            ViewType Type,
            uint32_t RootIndex,
            IBuffer* Resource) override;

        void SetDescriptorTable(
            uint32_t            RootIndex,
            GpuDescriptorHandle Handle) override;

    public:
        void Dispatch(
            size_t GroupCountX = 1,
            size_t GroupCountY = 1,
            size_t GroupCountZ = 1) override;
    };

    //

    class Dx12CopyCommandList final : public virtual ICopyCommandList,
                                      public Dx12CommandList
    {
    };
} // namespace Neon::RHI