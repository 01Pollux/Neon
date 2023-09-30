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
        void RecreateGridFrustum();

    private:
        Ptr<RHI::IRootSignature> m_GridFrustumRootSignature;
        Ptr<RHI::IPipelineState> m_GridFrustumPipelineState;
    };
} // namespace Neon::RG