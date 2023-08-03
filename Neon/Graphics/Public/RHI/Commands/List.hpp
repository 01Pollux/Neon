#pragma once

#include <RHI/Commands/Common.hpp>
#include <RHI/Resource/Common.hpp>
#include <Math/Common.hpp>
#include <Math/Viewport.hpp>
#include <Math/Colors.hpp>
#include <Math/Rect.hpp>
#include <array>
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
            const Vector3I&            DstPosition,
            const TextureCopyLocation& Src,
            const CopyBox*             SrcBox = nullptr) = 0;
    };

    //

    class ICommonCommandList : public virtual ICommandList
    {
    public:
        enum class ViewType : uint8_t
        {
            Cbv,
            Srv,
            Uav
        };

        /// <summary>
        /// Set root signature
        /// </summary>
        virtual void SetRootSignature(
            const Ptr<IRootSignature>& RootSig) = 0;

        /// <summary>
        /// Set pipeline state
        /// </summary>
        virtual void SetPipelineState(
            const Ptr<IPipelineState>& State) = 0;

    public:
        /// <summary>
        /// Sets constants in root signature
        /// </summary>
        virtual void SetConstants(
            uint32_t    RootIndex,
            const void* Constants,
            size_t      NumConstants32Bit,
            size_t      DestOffset = 0) = 0;

        /// <summary>
        /// Set resource view in root signature
        /// </summary>
        virtual void SetResourceView(
            ViewType          Type,
            uint32_t          RootIndex,
            GpuResourceHandle Handle) = 0;

        /// <summary>
        /// Allocate from dynamic buffer and set resource view in root signature
        /// </summary>
        virtual void SetDynamicResourceView(
            ViewType    Type,
            uint32_t    RootIndex,
            const void* Data,
            size_t      Size) = 0;

        /// <summary>
        /// Set descriptor table in root signature
        /// </summary>
        virtual void SetDescriptorTable(
            uint32_t            RootIndex,
            GpuDescriptorHandle Handle) = 0;
    };

    //

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
        /// Clear depth stencil view
        /// </summary>
        virtual void ClearDsv(
            const CpuDescriptorHandle& RtvHandle,
            std::optional<float>       Depth   = std::nullopt,
            std::optional<uint8_t>     Stencil = std::nullopt) = 0;

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
    };

    //

    class IComputeCommandList : public virtual ICommonCommandList
    {
    public:
        /// <summary>
        /// Dispatch compute shader threads
        /// </summary>
        virtual void Dispatch(
            size_t GroupCountX = 1,
            size_t GroupCountY = 1,
            size_t GroupCountZ = 1) = 0;

        /// <summary>
        /// Dispatch compute shader threads
        /// </summary>
        void Dispatch1D(
            size_t ThreadCountX,
            size_t GroupSizeX = 64)
        {
            Dispatch(
                Math::DivideByMultiple(ThreadCountX, GroupSizeX),
                1,
                1);
        }

        /// <summary>
        /// Dispatch compute shader threads
        /// </summary>
        void Dispatch2D(
            size_t ThreadCountX,
            size_t ThreadCountY,
            size_t GroupSizeX = 8,
            size_t GroupSizeY = 8)
        {
            Dispatch(
                Math::DivideByMultiple(ThreadCountX, GroupSizeX),
                Math::DivideByMultiple(ThreadCountY, GroupSizeY),
                1);
        }

        /// <summary>
        /// Dispatch compute shader threads
        /// </summary>
        void Dispatch3D(
            size_t ThreadCountX,
            size_t ThreadCountY,
            size_t ThreadCountZ,
            size_t GroupSizeX,
            size_t GroupSizeY,
            size_t GroupSizeZ)
        {

            Dispatch(
                Math::DivideByMultiple(ThreadCountX, GroupSizeX),
                Math::DivideByMultiple(ThreadCountY, GroupSizeY),
                Math::DivideByMultiple(ThreadCountZ, GroupSizeZ));
        }
    };

    //

    class ICopyCommandList : public virtual ICommandList
    {
    public:
    };
} // namespace Neon::RHI
