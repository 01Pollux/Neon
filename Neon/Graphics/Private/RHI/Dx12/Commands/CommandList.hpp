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
    };

    class Dx12GraphicsCommandList final : public virtual IGraphicsCommandList,
                                          public Dx12CommonCommandList

    {
    public:
        using Dx12CommonCommandList::Dx12CommonCommandList;

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
    };
} // namespace Neon::RHI