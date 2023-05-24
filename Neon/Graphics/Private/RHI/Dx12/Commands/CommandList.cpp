#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/Commands.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <Private/RHI/Dx12/RootSignature.hpp>
#include <Private/RHI/Dx12/PipelineState.hpp>
#include <Private/RHI/Dx12/Resource/Common.hpp>
#include <Math/Colors.hpp>

#include <algorithm>

namespace Neon::RHI
{
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
        IPipelineState* State)
    {
        m_CommandList->SetPipelineState(static_cast<Dx12PipelineState*>(State)->Get());
    }

    //

    void Dx12GraphicsCommandList::SetRootSignature(
        IRootSignature* RootSig)
    {
        m_CommandList->SetGraphicsRootSignature(static_cast<Dx12RootSignature*>(RootSig)->Get());
    }

    void Dx12GraphicsCommandList::ClearRtv(
        const CpuDescriptorHandle& RtvHandle,
        const Color4&              Color)
    {
        m_CommandList->ClearRenderTargetView({ RtvHandle.Value }, Color.data(), 0, nullptr);
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

} // namespace Neon::RHI