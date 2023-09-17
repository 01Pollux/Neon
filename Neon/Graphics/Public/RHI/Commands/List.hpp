#pragma once

#include <RHI/Commands/Common.hpp>
#include <RHI/Resource/Common.hpp>

#include <Math/Common.hpp>
#include <Math/Viewport.hpp>
#include <Math/Colors.hpp>
#include <Math/Rect.hpp>

#include <array>
#include <optional>
#include <span>

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

    namespace Views
    {
        class Index;
        class Vertex;
    } // namespace Views

    //

    struct DrawIndexArgs
    {
        // The location of the first index read by the GPU from the index buffer
        uint32_t StartIndex = 0;
        // A value added to each index before reading a vertex from the vertex buffer
        int32_t StartVertex = 0;
        // A value added to each index before reading per-instance data from a vertex buffer
        uint32_t StartInstance = 0;

        // Number of indices read from the index buffer for each instance
        uint32_t IndexCountPerInstance = 0;
        // Number of instances to draw
        uint32_t InstanceCount = 1;
    };

    struct DrawArgs
    {
        // Index of the first vertex
        int32_t StartVertex = 0;
        // A value added to each index before reading per-instance data from a vertex buffer
        uint32_t StartInstance = 0;

        // Number of vertices to draw
        uint32_t VertexCountPerInstance = 0;
        // Number of instances to draw
        uint32_t InstanceCount = 1;
    };

    //

    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;

        /// <summary>
        /// Begin event marker
        /// </summary>
        virtual void BeginEvent(
            const StringU8& Text,
            const Color4&   Color = Colors::White) = 0;

        /// <summary>
        /// Mark an entry
        /// </summary>
        virtual void MarkEvent(
            const StringU8& Text,
            const Color4&   Color = Colors::White) = 0;

        /// <summary>
        /// End event marker
        /// </summary>
        virtual void EndEvent() = 0;

    public:
        /// <summary>
        /// Copy subresources into the buffer
        /// </summary>
        virtual void CopySubresources(
            IGpuResource*                    DstResource,
            IGpuResource*                    Intermediate,
            size_t                           IntOffset,
            uint32_t                         FirstSubresource,
            std::span<const SubresourceDesc> SubResources) = 0;

        /// <summary>
        /// Copy Resource into the buffer
        /// </summary>
        virtual void CopyResource(
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
            const Vector3I&            DstPosition,
            const TextureCopyLocation& Src,
            const CopyBox*             SrcBox = nullptr) = 0;

        /// <summary>
        /// Insert an UAV barrier for specificed resources
        /// </summary>
        virtual void InsertUAVBarrier(
            std::span<RHI::IGpuResource*> Resources) = 0;

        /// <summary>
        /// Insert an UAV barrier for specificed resource
        /// </summary>
        void InsertUAVBarrier(
            RHI::IGpuResource* Resource)
        {
            InsertUAVBarrier({ &Resource, 1 });
        }

    public:
        /// <summary>
        /// Set root signature
        /// </summary>
        virtual void SetRootSignature(
            bool                       IsDirect,
            const Ptr<IRootSignature>& RootSig) = 0;

        /// <summary>
        /// Set pipeline state
        /// </summary>
        virtual void SetPipelineState(
            const Ptr<IPipelineState>& State) = 0;

    public:
        /// <summary>
        /// Allocate from dynamic buffer and set resource view in root signature
        /// </summary>
        virtual void SetDynamicResourceView(
            bool                IsDirect,
            CstResourceViewType Type,
            uint32_t            RootIndex,
            const void*         Data,
            size_t              Size) = 0;

        /// <summary>
        /// Sets constants in root signature
        /// </summary>
        virtual void SetConstants(
            bool        IsDirect,
            uint32_t    RootIndex,
            const void* Constants,
            size_t      NumConstants32Bit,
            size_t      DestOffset = 0) = 0;

        /// <summary>
        /// Sets constants in root signature
        /// </summary>
        template<typename _Ty>
        void SetConstants(
            bool       IsDirect,
            uint32_t   RootIndex,
            const _Ty& Constants,
            size_t     DestOffset = 0)
        {
            SetConstants(
                IsDirect,
                RootIndex,
                &Constants,
                sizeof(_Ty) / sizeof(uint32_t),
                DestOffset);
        }

        /// <summary>
        /// Set resource view in root signature
        /// </summary>
        virtual void SetResourceView(
            bool                IsDirect,
            CstResourceViewType Type,
            uint32_t            RootIndex,
            GpuResourceHandle   Handle) = 0;

        /// <summary>
        /// Set descriptor table in root signature
        /// </summary>
        virtual void SetDescriptorTable(
            bool                IsDirect,
            uint32_t            RootIndex,
            GpuDescriptorHandle Handle) = 0;

    public:
        /// <summary>
        /// Clear render target view
        /// </summary>
        virtual void ClearRtv(
            CpuDescriptorHandle RtvHandle,
            const Color4&       Color) = 0;

        /// <summary>
        /// Clear depth stencil view
        /// </summary>
        virtual void ClearDsv(
            CpuDescriptorHandle    RtvHandle,
            std::optional<float>   Depth   = std::nullopt,
            std::optional<uint8_t> Stencil = std::nullopt) = 0;

        /// <summary>
        /// Bind rtv/dsv to pipeline
        /// </summary>
        virtual void SetRenderTargets(
            CpuDescriptorHandle        ContiguousRtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) = 0;

        /// <summary>
        /// Bind rtv/dsv to pipeline
        /// </summary>
        virtual void SetRenderTargets(
            const CpuDescriptorHandle* Rtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr) = 0;

    public:
        /// <summary>
        /// Set scissor rects
        /// </summary>
        virtual void SetScissorRect(
            std::span<RectT<Vector2>> Scissors) = 0;

        /// <summary>
        /// Set scissor rect
        /// </summary>
        void SetScissorRect(
            const RectT<Vector2>& Scissor)
        {
            std::array<RectT<Vector2>, 1> Scissors{ Scissor };
            SetScissorRect(Scissors);
        }

        /// <summary>
        /// Set viewports
        /// </summary>
        virtual void SetViewport(
            std::span<ViewportF> Views) = 0;

        /// <summary>
        /// Set viewport
        /// </summary>
        void SetViewport(
            const ViewportF& View)
        {
            std::array<ViewportF, 1> Views{ View };
            SetViewport(Views);
        }

        /// <summary>
        /// Set primitive topology
        /// </summary>
        virtual void SetPrimitiveTopology(
            PrimitiveTopology Topology) = 0;

        /// <summary>
        /// Set index buffer view
        /// </summary>
        virtual void SetIndexBuffer(
            const Views::Index& View) = 0;

        /// <summary>
        /// Set vertex buffer views
        /// </summary>
        virtual void SetVertexBuffer(
            size_t               StartSlot,
            const Views::Vertex& Views) = 0;

    public:
        /// <summary>
        /// Draw indexed primitives
        /// </summary>
        virtual void Draw(
            const DrawIndexArgs& Args) = 0;

        /// <summary>
        /// Draw non-indexed primitives
        /// </summary>
        virtual void Draw(
            const DrawArgs& Args) = 0;

    public:
        /// <summary>
        /// Dispatch compute shader threads
        /// </summary>
        virtual void Dispatch(
            size_t GroupCountX = 1,
            size_t GroupCountY = 1,
            size_t GroupCountZ = 1) = 0;
    };

    //

    class GraphicsCommandList
    {
    public:
        GraphicsCommandList(
            ICommandList* CmdList) :
            m_CmdList(CmdList)
        {
        }

        /// <summary>
        /// Get command list
        /// </summary>
        [[nodiscard]] ICommandList* Get() const noexcept
        {
            return m_CmdList;
        }

        operator ICommandList*() const noexcept
        {
            return m_CmdList;
        }

        /// <summary>
        /// Copy subresources into the buffer
        /// </summary>
        void CopySubresources(
            IGpuResource*                    DstResource,
            IGpuResource*                    Intermediate,
            size_t                           IntOffset,
            uint32_t                         FirstSubresource,
            std::span<const SubresourceDesc> SubResources)
        {
            m_CmdList->CopySubresources(
                DstResource,
                Intermediate,
                IntOffset,
                FirstSubresource,
                SubResources);
        }

        /// <summary>
        /// Copy Resource into the buffer
        /// </summary>
        void CopyResource(
            IGpuResource* DstResource,
            IGpuResource* SrcResource)
        {
            m_CmdList->CopyResource(
                DstResource,
                SrcResource);
        }

        /// <summary>
        /// Copy buffer resource
        /// </summary>
        void CopyBufferRegion(
            IBuffer* DstBuffer,
            size_t   DstOffset,
            IBuffer* SrcBuffer,
            size_t   SrcOffset,
            size_t   NumBytes)
        {
            m_CmdList->CopyBufferRegion(
                DstBuffer,
                DstOffset,
                SrcBuffer,
                SrcOffset,
                NumBytes);
        }

        /// <summary>
        /// Copy texture resource
        /// </summary>
        void CopyTextureRegion(
            const TextureCopyLocation&   Dst,
            const Vector3I&              DstPosition,
            const TextureCopyLocation&   Src,
            const ICommandList::CopyBox* SrcBox = nullptr)
        {
            m_CmdList->CopyTextureRegion(
                Dst,
                DstPosition,
                Src,
                SrcBox);
        }

        /// <summary>
        /// Insert an UAV barrier for specificed resources
        /// </summary>
        void InsertUAVBarrier(
            std::span<RHI::IGpuResource*> Resources)
        {
            m_CmdList->InsertUAVBarrier(Resources);
        }

        /// <summary>
        /// Insert an UAV barrier for specificed resource
        /// </summary>
        void InsertUAVBarrier(
            RHI::IGpuResource* Resources)
        {
            m_CmdList->InsertUAVBarrier(Resources);
        }

    public:
        /// <summary>
        /// Set root signature
        /// </summary>
        void SetRootSignature(
            const Ptr<IRootSignature>& RootSig)
        {
            m_CmdList->SetRootSignature(
                true,
                RootSig);
        }

        /// <summary>
        /// Set pipeline state
        /// </summary>
        void SetPipelineState(
            const Ptr<IPipelineState>& State)
        {
            m_CmdList->SetPipelineState(
                State);
        }

    public:
        /// <summary>
        /// Allocate from dynamic buffer and set resource view in root signature
        /// </summary>
        void SetDynamicResourceView(
            CstResourceViewType Type,
            uint32_t            RootIndex,
            const void*         Data,
            size_t              Size)
        {
            m_CmdList->SetDynamicResourceView(
                true,
                Type,
                RootIndex,
                Data,
                Size);
        }

        /// <summary>
        /// Sets constants in root signature
        /// </summary>
        void SetConstants(
            uint32_t    RootIndex,
            const void* Constants,
            size_t      NumConstants32Bit,
            size_t      DestOffset = 0)
        {
            m_CmdList->SetConstants(
                true,
                RootIndex,
                Constants,
                NumConstants32Bit,
                DestOffset);
        }

        /// <summary>
        /// Sets constants in root signature
        /// </summary>
        template<typename _Ty>
        void SetConstants(
            uint32_t   RootIndex,
            const _Ty& Constants,
            size_t     DestOffset = 0)
        {
            m_CmdList->SetConstants<_Ty>(
                true,
                RootIndex,
                Constants,
                DestOffset);
        }

        /// <summary>
        /// Set resource view in root signature
        /// </summary>
        void SetResourceView(
            bool                IsDirect,
            CstResourceViewType Type,
            uint32_t            RootIndex,
            GpuResourceHandle   Handle)
        {
            m_CmdList->SetResourceView(
                true,
                Type,
                RootIndex,
                Handle);
        }

        /// <summary>
        /// Set descriptor table in root signature
        /// </summary>
        void SetDescriptorTable(
            uint32_t            RootIndex,
            GpuDescriptorHandle Handle)
        {
            m_CmdList->SetDescriptorTable(
                true,
                RootIndex,
                Handle);
        }

    public:
        /// <summary>
        /// Clear render target view
        /// </summary>
        void ClearRtv(
            CpuDescriptorHandle RtvHandle,
            const Color4&       Color)
        {
            m_CmdList->ClearRtv(
                RtvHandle,
                Color);
        }

        /// <summary>
        /// Clear depth stencil view
        /// </summary>
        void ClearDsv(
            CpuDescriptorHandle    RtvHandle,
            std::optional<float>   Depth   = std::nullopt,
            std::optional<uint8_t> Stencil = std::nullopt)
        {
            m_CmdList->ClearDsv(
                RtvHandle,
                Depth,
                Stencil);
        }

        /// <summary>
        /// Bind rtv/dsv to pipeline
        /// </summary>
        void SetRenderTargets(
            CpuDescriptorHandle        ContiguousRtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr)
        {
            m_CmdList->SetRenderTargets(
                &ContiguousRtvs,
                RenderTargetCount,
                DepthStencil);
        }

        /// <summary>
        /// Bind rtv/dsv to pipeline
        /// </summary>
        void SetRenderTargets(
            const CpuDescriptorHandle* Rtvs,
            size_t                     RenderTargetCount = 0,
            const CpuDescriptorHandle* DepthStencil      = nullptr)
        {
            m_CmdList->SetRenderTargets(
                Rtvs,
                RenderTargetCount,
                DepthStencil);
        }

    public:
        /// <summary>
        /// Set scissor rects
        /// </summary>
        void SetScissorRect(
            std::span<RectT<Vector2>> Scissors)
        {
            m_CmdList->SetScissorRect(
                Scissors);
        }

        /// <summary>
        /// Set scissor rect
        /// </summary>
        void SetScissorRect(
            const RectT<Vector2>& Scissor)
        {
            m_CmdList->SetScissorRect(
                Scissor);
        }

        /// <summary>
        /// Set viewports
        /// </summary>
        void SetViewport(
            std::span<ViewportF> Views)
        {
            m_CmdList->SetViewport(
                Views);
        }

        /// <summary>
        /// Set viewport
        /// </summary>
        void SetViewport(
            const ViewportF& View)
        {
            m_CmdList->SetViewport(
                View);
        }

        /// <summary>
        /// Set primitive topology
        /// </summary>
        void SetPrimitiveTopology(
            PrimitiveTopology Topology)
        {
            m_CmdList->SetPrimitiveTopology(
                Topology);
        }

        /// <summary>
        /// Set index buffer view
        /// </summary>
        void SetIndexBuffer(
            const Views::Index& View)
        {
            m_CmdList->SetIndexBuffer(
                View);
        }

        /// <summary>
        /// Set vertex buffer views
        /// </summary>
        void SetVertexBuffer(
            size_t               StartSlot,
            const Views::Vertex& Views)
        {
            m_CmdList->SetVertexBuffer(
                StartSlot,
                Views);
        }

    public:
        /// <summary>
        /// Draw indexed primitives
        /// </summary>
        void Draw(
            const DrawIndexArgs& Args)
        {
            m_CmdList->Draw(
                Args);
        }

        /// <summary>
        /// Draw non-indexed primitives
        /// </summary>
        void Draw(
            const DrawArgs& Args)
        {
            m_CmdList->Draw(
                Args);
        }

    private:
        ICommandList* m_CmdList = nullptr;
    };

    //

    class ComputeCommandList
    {
    public:
        ComputeCommandList(
            ICommandList* CmdList) :
            m_CmdList(CmdList)
        {
        }

        /// <summary>
        /// Get command list
        /// </summary>
        [[nodiscard]] ICommandList* Get() const noexcept
        {
            return m_CmdList;
        }

        operator ICommandList*() const noexcept
        {
            return m_CmdList;
        }

        /// <summary>
        /// Copy subresources into the buffer
        /// </summary>
        void CopySubresources(
            IGpuResource*                    DstResource,
            IGpuResource*                    Intermediate,
            size_t                           IntOffset,
            uint32_t                         FirstSubresource,
            std::span<const SubresourceDesc> SubResources)
        {
            m_CmdList->CopySubresources(
                DstResource,
                Intermediate,
                IntOffset,
                FirstSubresource,
                SubResources);
        }

        /// <summary>
        /// Copy Resource into the buffer
        /// </summary>
        void CopyResource(
            IGpuResource* DstResource,
            IGpuResource* SrcResource)
        {
            m_CmdList->CopyResource(
                DstResource,
                SrcResource);
        }

        /// <summary>
        /// Copy buffer resource
        /// </summary>
        void CopyBufferRegion(
            IBuffer* DstBuffer,
            size_t   DstOffset,
            IBuffer* SrcBuffer,
            size_t   SrcOffset,
            size_t   NumBytes)
        {
            m_CmdList->CopyBufferRegion(
                DstBuffer,
                DstOffset,
                SrcBuffer,
                SrcOffset,
                NumBytes);
        }

        /// <summary>
        /// Copy texture resource
        /// </summary>
        void CopyTextureRegion(
            const TextureCopyLocation&   Dst,
            const Vector3I&              DstPosition,
            const TextureCopyLocation&   Src,
            const ICommandList::CopyBox* SrcBox = nullptr)
        {
            m_CmdList->CopyTextureRegion(
                Dst,
                DstPosition,
                Src,
                SrcBox);
        }

        /// <summary>
        /// Insert an UAV barrier for specificed resources
        /// </summary>
        void InsertUAVBarrier(
            std::span<RHI::IGpuResource*> Resources)
        {
            m_CmdList->InsertUAVBarrier(Resources);
        }

        /// <summary>
        /// Insert an UAV barrier for specificed resource
        /// </summary>
        void InsertUAVBarrier(
            RHI::IGpuResource* Resources)
        {
            m_CmdList->InsertUAVBarrier(Resources);
        }

    public:
        /// <summary>
        /// Set root signature
        /// </summary>
        void SetRootSignature(
            const Ptr<IRootSignature>& RootSig)
        {
            m_CmdList->SetRootSignature(
                false,
                RootSig);
        }

        /// <summary>
        /// Set pipeline state
        /// </summary>
        void SetPipelineState(
            const Ptr<IPipelineState>& State)
        {
            m_CmdList->SetPipelineState(
                State);
        }

    public:
        /// <summary>
        /// Allocate from dynamic buffer and set resource view in root signature
        /// </summary>
        void SetDynamicResourceView(
            CstResourceViewType Type,
            uint32_t            RootIndex,
            const void*         Data,
            size_t              Size)
        {
            m_CmdList->SetDynamicResourceView(
                false,
                Type,
                RootIndex,
                Data,
                Size);
        }

        /// <summary>
        /// Sets constants in root signature
        /// </summary>
        void SetConstants(
            uint32_t    RootIndex,
            const void* Constants,
            size_t      NumConstants32Bit,
            size_t      DestOffset = 0)
        {
            m_CmdList->SetConstants(
                false,
                RootIndex,
                Constants,
                NumConstants32Bit,
                DestOffset);
        }

        /// <summary>
        /// Sets constants in root signature
        /// </summary>
        template<typename _Ty>
        void SetConstants(
            uint32_t   RootIndex,
            const _Ty& Constants,
            size_t     DestOffset = 0)
        {
            m_CmdList->SetConstants<_Ty>(
                false,
                RootIndex,
                Constants,
                DestOffset);
        }

        /// <summary>
        /// Set resource view in root signature
        /// </summary>
        void SetResourceView(
            CstResourceViewType Type,
            uint32_t            RootIndex,
            GpuResourceHandle   Handle)
        {
            m_CmdList->SetResourceView(
                false,
                Type,
                RootIndex,
                Handle);
        }

        /// <summary>
        /// Set descriptor table in root signature
        /// </summary>
        void SetDescriptorTable(
            uint32_t            RootIndex,
            GpuDescriptorHandle Handle)
        {
            m_CmdList->SetDescriptorTable(
                false,
                RootIndex,
                Handle);
        }

        /// <summary>
        /// Dispatch compute shader threads
        /// </summary>
        void Dispatch(
            size_t GroupCountX = 1,
            size_t GroupCountY = 1,
            size_t GroupCountZ = 1)
        {
            m_CmdList->Dispatch(
                GroupCountX,
                GroupCountY,
                GroupCountZ);
        }

    private:
        ICommandList* m_CmdList = nullptr;
    };

    //

    class CopyCommandList
    {
    public:
        CopyCommandList(
            ICommandList* CmdList) :
            m_CmdList(CmdList)
        {
        }

        /// <summary>
        /// Get command list
        /// </summary>
        [[nodiscard]] ICommandList* Get() const noexcept
        {
            return m_CmdList;
        }

        operator ICommandList*() const noexcept
        {
            return m_CmdList;
        }

        /// <summary>
        /// Copy subresources into the buffer
        /// </summary>
        void CopySubresources(
            IGpuResource*                    DstResource,
            IGpuResource*                    Intermediate,
            size_t                           IntOffset,
            uint32_t                         FirstSubresource,
            std::span<const SubresourceDesc> SubResources)
        {
            m_CmdList->CopySubresources(
                DstResource,
                Intermediate,
                IntOffset,
                FirstSubresource,
                SubResources);
        }

        /// <summary>
        /// Copy Resource into the buffer
        /// </summary>
        void CopyResource(
            IGpuResource* DstResource,
            IGpuResource* SrcResource)
        {
            m_CmdList->CopyResource(
                DstResource,
                SrcResource);
        }

        /// <summary>
        /// Copy buffer resource
        /// </summary>
        void CopyBufferRegion(
            IBuffer* DstBuffer,
            size_t   DstOffset,
            IBuffer* SrcBuffer,
            size_t   SrcOffset,
            size_t   NumBytes)
        {
            m_CmdList->CopyBufferRegion(
                DstBuffer,
                DstOffset,
                SrcBuffer,
                SrcOffset,
                NumBytes);
        }

        /// <summary>
        /// Copy texture resource
        /// </summary>
        void CopyTextureRegion(
            const TextureCopyLocation&   Dst,
            const Vector3I&              DstPosition,
            const TextureCopyLocation&   Src,
            const ICommandList::CopyBox* SrcBox = nullptr)
        {
            m_CmdList->CopyTextureRegion(
                Dst,
                DstPosition,
                Src,
                SrcBox);
        }

    private:
        ICommandList* m_CmdList = nullptr;
    };
} // namespace Neon::RHI
