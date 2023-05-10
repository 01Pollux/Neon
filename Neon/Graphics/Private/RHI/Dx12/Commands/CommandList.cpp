#include <GraphicsPCH.hpp>
#include <Private/RHI/Dx12/Commands/CommandList.hpp>
#include <Private/RHI/Dx12/Commands/CommandAllocator.hpp>
#include <Private/RHI/Dx12/Device.hpp>

#include <Private/RHI/Dx12/Commands/Commands.hpp>

#include <RHI/Resource/Common.hpp>
#include <Math/Colors.hpp>

#include <algorithm>

namespace Neon::RHI
{
    ICommandList* ICommandList::Create(
        ICommandAllocator* Allocator,
        CommandQueueType   Type)
    {
        return NEON_NEW Dx12CommandList(Allocator, Type);
    }

    Dx12CommandList::Dx12CommandList(
        ICommandAllocator* Allocator,
        CommandQueueType   Type)
    {
        auto Dx12Device = Dx12RenderDevice::Get()->GetDevice();
        ThrowIfFailed(Dx12Device->CreateCommandList(
            0,
            CastCommandQueueType(Type),
            static_cast<Dx12CommandAllocator*>(Allocator)->Get(),
            nullptr,
            IID_PPV_ARGS(&m_CommandList)));
        ThrowIfFailed(m_CommandList->Close());
    }

    void Dx12CommandList::Reset(
        ICommandAllocator* Allocator)
    {
        ThrowIfFailed(m_CommandList->Reset(static_cast<Dx12CommandAllocator*>(Allocator)->Get(), nullptr));
    }

    void Dx12CommandList::Close()
    {
        ThrowIfFailed(m_CommandList->Close());
    }

    ID3D12GraphicsCommandList* Dx12CommandList::Get()
    {
        return m_CommandList.Get();
    }

    //

    void Dx12GraphicsCommandList::ClearRtv(
        const CpuDescriptorHandle& RtvHandle,
        const Color4&              Color)
    {
        m_CommandList->ClearRenderTargetView({ RtvHandle.Handle }, Color.data(), 0, nullptr);
    }

    void Dx12GraphicsCommandList::SetRenderTargets(
        const CpuDescriptorHandle& ContiguousRtvs,
        size_t                     RenderTargetCount,
        const CpuDescriptorHandle* DepthStencil)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE Rtv(ContiguousRtvs.Handle);
        D3D12_CPU_DESCRIPTOR_HANDLE Dsv;
        if (DepthStencil)
        {
            Dsv = { DepthStencil->Handle };
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
        std::transform(Rtvs, Rtvs + RenderTargetCount, std::back_inserter(Rtv),
                       [](const CpuDescriptorHandle& Rtv) -> D3D12_CPU_DESCRIPTOR_HANDLE
                       { return { Rtv.Handle }; });

        D3D12_CPU_DESCRIPTOR_HANDLE Dsv;
        if (DepthStencil)
        {
            Dsv = { DepthStencil->Handle };
        }

        m_CommandList->OMSetRenderTargets(
            UINT(RenderTargetCount),
            Rtv.data(),
            TRUE,
            DepthStencil ? &Dsv : nullptr);
    }
} // namespace Neon::RHI