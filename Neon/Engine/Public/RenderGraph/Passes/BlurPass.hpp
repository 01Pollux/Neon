#pragma once

#include <RenderGraph/Pass.hpp>
#include <RenderGraph/Graph.hpp>

namespace Neon::Renderer
{
    class IMaterial;
}

namespace Neon::RG
{
    class BlurPass : public ComputeRenderPass<BlurPass>
    {
        friend class RenderPass;

    public:
        struct BlurPassData
        {
            String ViewName;

            ResourceId Source;
            ResourceId Destination;
        };

        BlurPass(
            BlurPassData Data);

    protected:
        void ResolveResources(
            ResourceResolver& Resolver) override;

        void DispatchTyped(
            const GraphStorage&     Storage,
            RHI::ComputeCommandList CommandList);

    private:
        BlurPassData m_Data;

        Ptr<RHI::IRootSignature> m_BlurPassRootSignature;
        Ptr<RHI::IPipelineState> m_BlurPassPipelineState;
    };
} // namespace Neon::RG