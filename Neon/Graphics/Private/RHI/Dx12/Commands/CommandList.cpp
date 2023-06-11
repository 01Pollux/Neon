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

namespace Neon::RHI
{
    Dx12CommandList::Dx12CommandList(
        ISwapchain* Swapchain) :
        m_Swapchain(static_cast<Dx12Swapchain*>(Swapchain))
    {
    }

    Dx12CommandList::~Dx12CommandList()
    {
    }

    void Dx12CommandList::CopySubresources(
        IGpuResource*              DstResource,
        IGpuResource*              Intermediate,
        size_t                     IntOffset,
        uint32_t                   FirstSubresource,
        std::span<SubresourceDesc> SubResources)
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

    void Dx12CommandList::CopyResources(
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
        const Vector3DI&           DstPosition,
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
            DstPosition.x(),
            DstPosition.y(),
            DstPosition.z(),
            &Dx12Src,
            SrcBox ? &Box : nullptr);
    }

    void Dx12CommandList::AttachCommandList(
        ID3D12GraphicsCommandList* CommandList)
    {
        m_CommandList = CommandList;
    }

    ID3D12GraphicsCommandList* Dx12CommandList::Get()
    {
        return m_CommandList;
    }

    //

    void Dx12CommonCommandList::SetPipelineState(
        const Ptr<IPipelineState>& State)
    {
        m_CommandList->SetPipelineState(static_cast<Dx12PipelineState*>(State.get())->Get());
    }

    //

    Views::Generic& Dx12CommonCommandList::GetResourceView()
    {
        if (!m_ResourceView)
        {
            ReserveDescriptorHeaps();
        }
        return m_ResourceView;
    }

    Views::Generic& Dx12CommonCommandList::GetSamplerView()
    {
        if (!m_SamplerView)
        {
            ReserveDescriptorHeaps();
        }
        return m_SamplerView;
    }

    void Dx12CommonCommandList::SetDynamicResourceView(
        ViewType    Type,
        uint32_t    RootIndex,
        const void* Data,
        size_t      Size)
    {
        const uint32_t Alignment =
            Type == ViewType::Cbv ? D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT : 1;

        UPtr<IUploadBuffer> Buffer{ IUploadBuffer::Create(m_Swapchain, { .Size = Size, .Alignment = Alignment }) };

        auto BufferData = Buffer->Map();
        std::copy_n(static_cast<const uint8_t*>(Data), Size, BufferData);
        Buffer->Unmap();

        SetResourceView(
            Type,
            RootIndex,
            Buffer.get());
    }

    void Dx12CommonCommandList::ReserveDescriptorHeaps()
    {
        std::array<ID3D12DescriptorHeap*, 2> OldHeaps{}, NewHeaps{};

        auto& ResourceView = m_ResourceView.GetHandle();
        auto& SamplerView  = m_SamplerView.GetHandle();

        OldHeaps[0] = ResourceView.Heap ? static_cast<Dx12DescriptorHeap*>(ResourceView.Heap)->Get() : nullptr;
        OldHeaps[1] = SamplerView.Heap ? static_cast<Dx12DescriptorHeap*>(SamplerView.Heap)->Get() : nullptr;

        if (uint32_t Count = m_RootSignature->GetResourceCountInDescriptor())
        {
            m_ResourceView = Views::Generic(
                m_ResourceViewAllocator.get(),
                Count);
        }
        if (uint32_t Count = m_RootSignature->GetSamplerCountInDescriptor())
        {
            m_SamplerView = Views::Generic(
                m_SamplerViewAllocator.get(),
                Count);
        }

        NewHeaps[0] = ResourceView.Heap ? static_cast<Dx12DescriptorHeap*>(ResourceView.Heap)->Get() : nullptr;
        NewHeaps[1] = SamplerView.Heap ? static_cast<Dx12DescriptorHeap*>(SamplerView.Heap)->Get() : nullptr;

        if (OldHeaps != NewHeaps)
        {
            m_CommandList->SetDescriptorHeaps(2, NewHeaps.data());
        }
    }

    void Dx12CommonCommandList::DiscardDescriptorHeaps()
    {
        if (m_ResourceView)
        {
            m_Swapchain->SafeRelease(m_ResourceViewAllocator.get(), m_ResourceView.GetHandle());
            m_ResourceView = {};
        }
        if (m_SamplerView)
        {
            m_Swapchain->SafeRelease(m_SamplerViewAllocator.get(), m_SamplerView.GetHandle());
            m_SamplerView = {};
        }
    }

    //

    void Dx12GraphicsCommandList::SetRootSignature(
        const Ptr<IRootSignature>& RootSig)
    {
        m_CommandList->SetGraphicsRootSignature(static_cast<Dx12RootSignature*>(RootSig.get())->Get());
        m_RootSignature = RootSig;
        DiscardDescriptorHeaps();
    }

    //

    void Dx12GraphicsCommandList::SetConstants(
        uint32_t    RootIndex,
        const void* Constants,
        size_t      NumConstants32Bit,
        size_t      DestOffset)
    {
        m_CommandList->SetGraphicsRoot32BitConstants(
            RootIndex,
            uint32_t(NumConstants32Bit),
            Constants,
            uint32_t(DestOffset));
    }

    void Dx12GraphicsCommandList::SetResourceView(
        ViewType Type,
        uint32_t RootIndex,
        IBuffer* Resource)
    {
        D3D12_GPU_VIRTUAL_ADDRESS Handle{ Resource->GetHandle().Value };
        switch (Type)
        {
        case ViewType::Cbv:
            m_CommandList->SetGraphicsRootConstantBufferView(
                RootIndex,
                Handle);
            break;
        case ViewType::Srv:
            m_CommandList->SetGraphicsRootShaderResourceView(
                RootIndex,
                Handle);
            break;
        case ViewType::Uav:
            m_CommandList->SetGraphicsRootUnorderedAccessView(
                RootIndex,
                Handle);
            break;
        }
    }

    void Dx12GraphicsCommandList::SetDescriptorTable(
        UINT                RootIndex,
        GpuDescriptorHandle Handle)
    {
        m_CommandList->SetGraphicsRootDescriptorTable(RootIndex, { Handle.Value });
    }

    //

    void Dx12GraphicsCommandList::ClearRtv(
        const CpuDescriptorHandle& RtvHandle,
        const Color4&              Color)
    {
        m_CommandList->ClearRenderTargetView({ RtvHandle.Value }, Color.data(), 0, nullptr);
    }

    void Dx12GraphicsCommandList::ClearDsv(
        const CpuDescriptorHandle& RtvHandle,
        std::optional<float>       Depth,
        std::optional<uint8_t>     Stencil)
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

    void Dx12GraphicsCommandList::SetRenderTargets(
        const CpuDescriptorHandle& ContiguousRtvs,
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

    void Dx12GraphicsCommandList::SetRenderTargets(
        const CpuDescriptorHandle* Rtvs,
        size_t                     RenderTargetCount,
        const CpuDescriptorHandle* DepthStencil)
    {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Rtv;
        Rtv.reserve(RenderTargetCount);

        std::transform(Rtvs, Rtvs + RenderTargetCount, std::back_inserter(Rtv),
                       [](const CpuDescriptorHandle& Rtv) -> D3D12_CPU_DESCRIPTOR_HANDLE
                       { return { Rtv.Value }; });

        D3D12_CPU_DESCRIPTOR_HANDLE Dsv;
        if (DepthStencil)
        {
            Dsv = { DepthStencil->Value };
        }

        m_CommandList->OMSetRenderTargets(
            UINT(RenderTargetCount),
            Rtv.data(),
            TRUE,
            DepthStencil ? &Dsv : nullptr);
    }

    void Dx12GraphicsCommandList::SetScissorRect(
        std::span<RectT<Vector2D>> Scissors)
    {
        std::vector<D3D12_RECT> Rects;
        Rects.reserve(Scissors.size());

        std::transform(
            Scissors.begin(),
            Scissors.end(),
            std::back_inserter(Rects),
            [](const RectT<Vector2D>& Rect) -> D3D12_RECT
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

    void Dx12GraphicsCommandList::SetViewport(
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

    void Dx12GraphicsCommandList::SetPrimitiveTopology(
        PrimitiveTopology Topology)
    {
        m_CommandList->IASetPrimitiveTopology(CastPrimitiveTopology(Topology));
    }

    void Dx12GraphicsCommandList::SetIndexBuffer(
        const Views::Index& View)
    {
        D3D12_INDEX_BUFFER_VIEW IndexBuffer{
            .BufferLocation = View.Get().Handle.Value,
            .SizeInBytes    = View.Get().Size,
            .Format         = DXGI_FORMAT_R16_UINT
        };
        m_CommandList->IASetIndexBuffer(&IndexBuffer);
    }

    void Dx12GraphicsCommandList::SetVertexBuffer(
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

    void Dx12GraphicsCommandList::Draw(
        const DrawIndexArgs& Args)
    {
        m_CommandList->DrawIndexedInstanced(
            Args.IndexCountPerInstance,
            Args.InstanceCount,
            Args.StartIndex,
            Args.StartVertex,
            Args.StartInstance);
        DiscardDescriptorHeaps();
    }

    void Dx12GraphicsCommandList::Draw(
        const DrawArgs& Args)
    {
        m_CommandList->DrawInstanced(
            Args.VertexCountPerInstance,
            Args.InstanceCount,
            Args.StartVertex,
            Args.StartInstance);
        DiscardDescriptorHeaps();
    }

    //

    void Dx12ComputeCommandList::SetRootSignature(
        const Ptr<IRootSignature>& RootSig)
    {
        m_CommandList->SetComputeRootSignature(static_cast<Dx12RootSignature*>(RootSig.get())->Get());
        m_RootSignature = RootSig;
        DiscardDescriptorHeaps();
    }

    void Dx12ComputeCommandList::SetConstants(
        uint32_t    RootIndex,
        const void* Constants,
        size_t      NumConstants32Bit,
        size_t      DestOffset)
    {
        m_CommandList->SetComputeRoot32BitConstants(
            RootIndex,
            uint32_t(NumConstants32Bit),
            Constants,
            uint32_t(DestOffset));
    }

    void Dx12ComputeCommandList::SetResourceView(
        ViewType Type,
        uint32_t RootIndex,
        IBuffer* Resource)
    {
        D3D12_GPU_VIRTUAL_ADDRESS Handle{ Resource->GetHandle().Value };
        switch (Type)
        {
        case ViewType::Cbv:
            m_CommandList->SetComputeRootConstantBufferView(
                RootIndex,
                Handle);
            break;
        case ViewType::Srv:
            m_CommandList->SetComputeRootShaderResourceView(
                RootIndex,
                Handle);
            break;
        case ViewType::Uav:
            m_CommandList->SetComputeRootUnorderedAccessView(
                RootIndex,
                Handle);
            break;
        }
    }

    void Dx12ComputeCommandList::SetDescriptorTable(
        UINT                RootIndex,
        GpuDescriptorHandle Handle)
    {
        m_CommandList->SetComputeRootDescriptorTable(RootIndex, { Handle.Value });
    }

    void Dx12ComputeCommandList::Dispatch(
        size_t GroupCountX,
        size_t GroupCountY,
        size_t GroupCountZ)
    {
        m_CommandList->Dispatch(
            UINT(GroupCountX),
            UINT(GroupCountY),
            UINT(GroupCountZ));
        DiscardDescriptorHeaps();
    }
} // namespace Neon::RHI