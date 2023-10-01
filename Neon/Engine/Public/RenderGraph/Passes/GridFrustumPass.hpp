#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class GridFrustumPass : public ComputeRenderPass<GridFrustumPass>
    {
        friend class RenderPass;

    public:
        GridFrustumPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        /// <summary>
        /// Recreates the grid frustum resource if needed. (window size changed)
        /// </summary>
        void RecreateGridFrustum(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

        /// <summary>
        /// Dispatches the grid frustum pass.
        /// </summary>
        void DispatchGridFrustum(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList,
            const Size2I&           GridCount);

    private:
        Size2I                    m_GridSize;
        UPtr<RHI::IGpuResource>   m_GridFrustum;
        RHI::DescriptorHeapHandle m_GridFrustumViews; // 0: UAV, 1: SRV
        Ptr<RHI::IRootSignature>  m_GridFrustumRS;
        Ptr<RHI::IPipelineState>  m_GridFrustumPSO;
    };
} // namespace Neon::RG