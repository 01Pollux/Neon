#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/Commands.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/Swapchain.hpp>

#include <RHI/Resource/Common.hpp>
#include <Math/Colors.hpp>

#include <algorithm>

namespace Neon::RHI
{
    Dx12CommandList::~Dx12CommandList()
    {
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