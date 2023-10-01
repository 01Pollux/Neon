#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class LightCullPass : public ComputeRenderPass<LightCullPass>
    {
        friend class RenderPass;

    public:
        LightCullPass();

        ~LightCullPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        /// <summary>
        /// Initialize root signature and pipeline state.
        /// </summary>
        void InitializeGridFrustumGen();

        /// <summary>
        /// Recreates the grid frustum resource if needed. (window size changed)
        /// </summary>
        void RecreateGridFrustumIfNeeded(
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

        Ptr<RHI::IRootSignature> m_LightCullRS;
        Ptr<RHI::IPipelineState> m_LightCullPSO;
    };
} // namespace Neon::RG