#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/Commands.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/PipelineState.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <RHI/Resource/Views/Shader.hpp>

#include <Math/Colors.hpp>
#include <Math/Rect.hpp>
#include <Math/Viewport.hpp>

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include <pix.h>

namespace Neon::RHI
{
#ifndef NEON_DIST
    void Encode_Pix3Blob(
        UINT64 (&Blob)[64],
        const char* Label,
        uint32_t    Size,
        uint64_t    Color)
    {

        Blob[0] = (0x2ull << 10); // PIXEvent_BeginEvent_NoArgs
        Blob[1] = Color;
        Blob[2] = (8ull /* copyChunkSize */ << 55) | (1ull /* isANSI */ << 54);
        std::fill_n(Blob + 3, 61, 0);
        Size = std::min(Size, uint32_t(sizeof(Blob) - (4 * sizeof(UINT64))));
        std::copy_n(Label, Size, std::bit_cast<char*>(Blob + 3));
    }
#endif

    void Dx12CommandList::BeginEvent(
        const StringU8& Text,
        const Color4&   Color)
    {
#ifndef NEON_DIST
        Encode_Pix3Blob(m_Pix3Blob, Text.c_str(), uint32_t(Text.size()), ColorToU32(Color));
        m_CommandList->BeginEvent(2, m_Pix3Blob, sizeof(m_Pix3Blob));
#endif
    }

    void Dx12CommandList::MarkEvent(
        const StringU8& Text,
        const Color4&   Color)
    {
#ifndef NEON_DIST
        Encode_Pix3Blob(m_Pix3Blob, Text.c_str(), uint32_t(Text.size()), ColorToU32(Color));
        m_CommandList->SetMarker(2, m_Pix3Blob, sizeof(m_Pix3Blob));
#endif
    }

    void Dx12CommandList::EndEvent()
    {
#ifndef NEON_DIST
        m_CommandList->EndEvent();
#endif
    }

    //

    void Dx12CommandList::CopySubresources(
        IGpuResource*                    DstResource,
        IGpuResource*                    Intermediate,
        size_t                           IntOffset,
        uint32_t                         FirstSubresource,
        std::span<const SubresourceDesc> SubResources)
    {
        std::vector<D3D12_SUBRESOURCE_DATA> Dx12Subresources;
        Dx12Subresources.reserve(SubResources.size());

        std::ranges::transform(
            SubResources,
            std::back_inserter(Dx12Subresources),
            [](const SubresourceDesc& Subresource)
            {
                return D3D12_SUBRESOURCE_DATA{
                    .pData      = Subresource.Data,
                    .RowPitch   = LONG_PTR(Subresource.RowPitch),
                    .SlicePitch = LONG_PTR(Subresource.SlicePitch),
                };
            });

        UpdateSubresources(
            m_CommandList,
            GetDx12Resource(DstResource),
            GetDx12Resource(Intermediate),
            IntOffset,
            FirstSubresource,
            UINT(Dx12Subresources.size()),
            Dx12Subresources.data());
    }

    void Dx12CommandList::CopyResource(
        IGpuResource* DstResource,
        IGpuResource* SrcResource)
    {
        m_CommandList->CopyResource(
            GetDx12Resource(DstResource),
            GetDx12Resource(SrcResource));
    }

    void Dx12CommandList::CopyBufferRegion(
        IBuffer* DstBuffer,
        size_t   DstOffset,
        IBuffer* SrcBuffer,
        size_t   SrcOffset,
        size_t   NumBytes)
    {
        m_CommandList->CopyBufferRegion(
            GetDx12Resource(DstBuffer),
            DstOffset,
            GetDx12Resource(SrcBuffer),
            SrcOffset,
            NumBytes);
    }

    void Dx12CommandList::CopyTextureRegion(
        const TextureCopyLocation& Dst,
        const Vector3I&            DstPosition,
        const TextureCopyLocation& Src,
        const CopyBox*             SrcBox)
    {
        auto GetCopyLocation = [](const TextureCopyLocation& Location)
        {
            if (std::holds_alternative<uint32_t>(Location.Subresource))
            {
                return D3D12_TEXTURE_COPY_LOCATION{
                    .pResource        = GetDx12Resource(Location.Resource),
                    .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                    .SubresourceIndex = std::get<uint32_t>(Location.Subresource),
                };
            }
            else
            {
                auto& Footprint = std::get<SubresourceFootprint>(Location.Subresource);
                return D3D12_TEXTURE_COPY_LOCATION{
                    .pResource       = GetDx12Resource(Location.Resource),
                    .Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                    .PlacedFootprint = {
                        .Offset    = Footprint.Offset,
                        .Footprint = {
                            .Format   = CastFormat(Footprint.Format),
                            .Width    = Footprint.Width,
                            .Height   = Footprint.Height,
                            .Depth    = Footprint.Depth,
                            .RowPitch = Footprint.RowPitch,
                        },
                    },
                };
            }
        };

        auto Dx12Dst = GetCopyLocation(Dst);
        auto Dx12Src = GetCopyLocation(Src);

        D3D12_BOX Box;
        if (SrcBox)
        {
            Box = {
                .left   = SrcBox->Left,
                .top    = SrcBox->Top,
                .front  = SrcBox->Front,
                .right  = SrcBox->Right,
                .bottom = SrcBox->Bottom,
                .back   = SrcBox->Back
            };
        }

        m_CommandList->CopyTextureRegion(
            &Dx12Dst,
            DstPosition.x,
            DstPosition.y,
            DstPosition.z,
            &Dx12Src,
            SrcBox ? &Box : nullptr);
    }

    //

    void Dx12CommandList::SetRootSignature(
        bool                       IsDirect,
        const Ptr<IRootSignature>& RootSig)
    {
        if (m_RootSignature != RootSig)
        {
            if (IsDirect)
            {
                m_CommandList->SetGraphicsRootSignature(static_cast<Dx12RootSignature*>(RootSig.get())->Get());
            }
            else
            {
                m_CommandList->SetComputeRootSignature(static_cast<Dx12RootSignature*>(RootSig.get())->Get());
            }
            m_RootSignature = RootSig;
        }
    }

    void Dx12CommandList::SetPipelineState(
        const Ptr<IPipelineState>& State)
    {
        if (m_PipelineState != State)
        {
            m_CommandList->SetPipelineState(static_cast<Dx12PipelineState*>(State.get())->Get());
            m_PipelineState = State;
        }
    }

    //

    void Dx12CommandList::SetDynamicResourceView(
        bool                IsDirect,
        CstResourceViewType Type,
        uint32_t            RootIndex,
        const void*         Data,
        size_t              Size)
    {
        const uint32_t Alignment =
            Type == CstResourceViewType::Cbv ? D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT : 1;

        IGlobalBufferPool::BufferType BufferType;
        switch (Type)
        {
        case CstResourceViewType::Cbv:
        case CstResourceViewType::Srv:
            BufferType = IGlobalBufferPool::BufferType::ReadWriteGPUR;
            break;
        case CstResourceViewType::Uav:
            BufferType = IGlobalBufferPool::BufferType::ReadWriteGPURW;
            break;
        }

        UBufferPoolHandle Buffer(
            Size,
            Alignment,
            BufferType);

        Buffer.AsUpload()->Write(0, Data, Size);

        SetResourceView(
            IsDirect,
            Type,
            RootIndex,
            Buffer.GetGpuHandle());
    }

    //

    void Dx12CommandList::SetConstants(
        bool        IsDirect,
        uint32_t    RootIndex,
        const void* Constants,
        size_t      NumConstants32Bit,
        size_t      DestOffset)
    {
        if (IsDirect)
        {
            m_CommandList->SetGraphicsRoot32BitConstants(
                RootIndex,
                uint32_t(NumConstants32Bit),
                Constants,
                uint32_t(DestOffset));
        }
        else
        {
            m_CommandList->SetComputeRoot32BitConstants(
                RootIndex,
                uint32_t(NumConstants32Bit),
                Constants,
                uint32_t(DestOffset));
        }
    }

    void Dx12CommandList::SetResourceView(
        bool                IsDirect,
        CstResourceViewType Type,
        uint32_t            RootIndex,
        GpuResourceHandle   Handle)
    {
        D3D12_GPU_VIRTUAL_ADDRESS Dx12Handle{ Handle.Value };
        switch (Type)
        {
        case CstResourceViewType::Cbv:
            if (IsDirect)
            {
                m_CommandList->SetGraphicsRootConstantBufferView(
                    RootIndex,
                    Dx12Handle);
            }
            else
            {
                m_CommandList->SetComputeRootConstantBufferView(
                    RootIndex,
                    Dx12Handle);
            }
            break;
        case CstResourceViewType::Srv:
            if (IsDirect)
            {
                m_CommandList->SetGraphicsRootShaderResourceView(
                    RootIndex,
                    Dx12Handle);
            }
            else
            {
                m_CommandList->SetComputeRootShaderResourceView(
                    RootIndex,
                    Dx12Handle);
            }
            break;
        case CstResourceViewType::Uav:
            if (IsDirect)
            {
                m_CommandList->SetGraphicsRootUnorderedAccessView(
                    RootIndex,
                    Dx12Handle);
            }
            else
            {
                m_CommandList->SetComputeRootUnorderedAccessView(
                    RootIndex,
                    Dx12Handle);
            }
            break;
        }
    }

    void Dx12CommandList::SetDescriptorTable(
        bool                IsDirect,
        UINT                RootIndex,
        GpuDescriptorHandle Handle)
    {
        if (IsDirect)
        {
            m_CommandList->SetGraphicsRootDescriptorTable(RootIndex, { Handle.Value });
        }
        else
        {
            m_CommandList->SetComputeRootDescriptorTable(RootIndex, { Handle.Value });
        }
    }

    //

    void Dx12CommandList::ClearRtv(
        CpuDescriptorHandle RtvHandle,
        const Color4&       Color)
    {
        m_CommandList->ClearRenderTargetView({ RtvHandle.Value }, glm::value_ptr(Color), 0, nullptr);
    }

    void Dx12CommandList::ClearDsv(
        CpuDescriptorHandle    RtvHandle,
        std::optional<float>   Depth,
        std::optional<uint8_t> Stencil)
    {
        D3D12_CLEAR_FLAGS ClearFlags = {};
        if (Depth)
        {
            ClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        }
        if (Stencil)
        {
            ClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        }
        m_CommandList->ClearDepthStencilView({ RtvHandle.Value }, ClearFlags, Depth.value_or(1.0f), Stencil.value_or(0), 0, nullptr);
    }

    void Dx12CommandList::SetRenderTargets(
        CpuDescriptorHandle        ContiguousRtvs,
        size_t                     RenderTargetCount,
        const CpuDescriptorHandle* DepthStencil)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE Rtv(ContiguousRtvs.Value);
        D3D12_CPU_DESCRIPTOR_HANDLE Dsv;
        if (DepthStencil)
        {
            Dsv = { DepthStencil->Value };
        }

        m_CommandList->OMSetRenderTargets(
            UINT(RenderTargetCount),
            &Rtv,
            TRUE,
            DepthStencil ? &Dsv : nullptr);
    }

    void Dx12CommandList::SetRenderTargets(
        const CpuDescriptorHandle* Rtvs,
        size_t                     RenderTargetCount,
        const CpuDescriptorHandle* DepthStencil)
    {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Rtv;
        Rtv.reserve(RenderTargetCount);

        std::transform(Rtvs, Rtvs + RenderTargetCount, std::back_inserter(Rtv),
                       [](CpuDescriptorHandle Rtv) -> D3D12_CPU_DESCRIPTOR_HANDLE
                       { return { Rtv.Value }; });

        D3D12_CPU_DESCRIPTOR_HANDLE Dsv;
        if (DepthStencil)
        {
            Dsv = { DepthStencil->Value };
        }

        m_CommandList->OMSetRenderTargets(
            UINT(RenderTargetCount),
            Rtv.data(),
            FALSE,
            DepthStencil ? &Dsv : nullptr);
    }

    //

    void Dx12CommandList::SetScissorRect(
        std::span<RectT<Vector2>> Scissors)
    {
        std::vector<D3D12_RECT> Rects;
        Rects.reserve(Scissors.size());

        std::transform(
            Scissors.begin(),
            Scissors.end(),
            std::back_inserter(Rects),
            [](const RectT<Vector2>& Rect) -> D3D12_RECT
            {
                return {
                    .left   = LONG(Rect.Left()),
                    .top    = LONG(Rect.Top()),
                    .right  = LONG(Rect.Right()),
                    .bottom = LONG(Rect.Bottom())
                };
            });

        m_CommandList->RSSetScissorRects(UINT(Rects.size()), Rects.data());
    }

    void Dx12CommandList::SetViewport(
        std::span<ViewportF> Views)
    {
        std::vector<D3D12_VIEWPORT> Viewports;
        Viewports.reserve(Views.size());

        std::transform(
            Views.begin(),
            Views.end(),
            std::back_inserter(Viewports),
            [](const ViewportF& View) -> D3D12_VIEWPORT
            {
                return {
                    .TopLeftX = View.TopLeftX,
                    .TopLeftY = View.TopLeftY,
                    .Width    = View.Width,
                    .Height   = View.Height,
                    .MinDepth = View.MinDepth,
                    .MaxDepth = View.MaxDepth
                };
            });

        m_CommandList->RSSetViewports(UINT(Viewports.size()), Viewports.data());
    }

    void Dx12CommandList::SetPrimitiveTopology(
        PrimitiveTopology Topology)
    {
        m_CommandList->IASetPrimitiveTopology(CastPrimitiveTopology(Topology));
    }

    void Dx12CommandList::SetIndexBuffer(
        const Views::Index& View)
    {
        D3D12_INDEX_BUFFER_VIEW IndexBuffer{
            .BufferLocation = View.Get().Handle.Value,
            .SizeInBytes    = View.Get().Size,
            .Format         = View.Get().Is32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT
        };
        m_CommandList->IASetIndexBuffer(&IndexBuffer);
    }

    void Dx12CommandList::SetVertexBuffer(
        size_t               StartSlot,
        const Views::Vertex& Views)
    {
        std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBuffers;
        VertexBuffers.reserve(Views.GetViews().size());

        std::transform(
            Views.GetViews().begin(),
            Views.GetViews().end(),
            std::back_inserter(VertexBuffers),
            [](const Views::Vertex::View& View) -> D3D12_VERTEX_BUFFER_VIEW
            {
                return {
                    .BufferLocation = View.Handle.Value,
                    .SizeInBytes    = View.Size,
                    .StrideInBytes  = View.Stride
                };
            });

        m_CommandList->IASetVertexBuffers(
            UINT(StartSlot),
            UINT(VertexBuffers.size()),
            VertexBuffers.data());
    }

    //

    void Dx12CommandList::Draw(
        const DrawIndexArgs& Args)
    {
        m_CommandList->DrawIndexedInstanced(
            Args.IndexCountPerInstance,
            Args.InstanceCount,
            Args.StartIndex,
            Args.StartVertex,
            Args.StartInstance);
    }

    void Dx12CommandList::Draw(
        const DrawArgs& Args)
    {
        m_CommandList->DrawInstanced(
            Args.VertexCountPerInstance,
            Args.InstanceCount,
            Args.StartVertex,
            Args.StartInstance);
    }

    //

    void Dx12CommandList::Dispatch(
        size_t GroupCountX,
        size_t GroupCountY,
        size_t GroupCountZ)
    {
        m_CommandList->Dispatch(
            UINT(GroupCountX),
            UINT(GroupCountY),
            UINT(GroupCountZ));
    }

    //

    void Dx12CommandList::Reset()
    {
        m_PipelineState = nullptr;
        m_RootSignature = nullptr;
    }

    //

    void Dx12CommandList::AttachCommandList(
        ID3D12GraphicsCommandList* CommandList)
    {
        m_CommandList = CommandList;
    }

    ID3D12GraphicsCommandList* Dx12CommandList::Get()
    {
        return m_CommandList;
    }
} // namespace Neon::RHI