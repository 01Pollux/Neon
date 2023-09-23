#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::RG
{
    class AmbientPass : public ComputeRenderPass<AmbientPass>
    {
        friend class RenderPass;

        struct ParamsType
        {
            uint32_t Color;
        };

    public:
        AmbientPass();

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        Ptr<RHI::IPipelineState> m_AmbientPipeline;
        Ptr<RHI::IRootSignature> m_AmbientRootSignature;
    };
} // namespace Neon::RG